#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libexif/exif-data.h>
#include <esp_log.h>
#include "metadata.h"

#define TAG "Metadata"


/* start of JPEG image data section */
static const unsigned int image_data_offset = 20;
#define image_data_len (image_jpg_len - image_data_offset)

/* raw EXIF header data */
static const unsigned char exif_header[] = {
  0xff, 0xd8, 0xff, 0xe1
};
/* length of data in exif_header */
static const unsigned int exif_header_len = sizeof(exif_header);

/* byte order to use in the EXIF block */
#define FILE_BYTE_ORDER EXIF_BYTE_ORDER_INTEL

/* comment to write into the EXIF block */
#define FILE_COMMENT "libexif demonstration image"

/* special header required for EXIF_TAG_USER_COMMENT */
#define ASCII_COMMENT "ASCII\0\0\0"

#define FILE_NAME "dummy.txt"

/* Get an existing tag, or create one if it doesn't exist */
static ExifEntry *init_tag(ExifData *exif, ExifIfd ifd, ExifTag tag)
{
	ExifEntry *entry;
	/* Return an existing tag if one exists */
	if (!((entry = exif_content_get_entry (exif->ifd[ifd], tag)))) {
	    /* Allocate a new entry */
	    entry = exif_entry_new ();
	    if (entry == NULL) { /* catch an out of memory condition */
			return NULL;
		}
	    entry->tag = tag; /* tag must be set before calling
				 exif_content_add_entry */

	    /* Attach the ExifEntry to an IFD */
	    exif_content_add_entry (exif->ifd[ifd], entry);

	    /* Allocate memory for the entry and fill with default data */
	    exif_entry_initialize (entry, tag);

	    /* Ownership of the ExifEntry has now been passed to the IFD.
	     * One must be very careful in accessing a structure after
	     * unref'ing it; in this case, we know "entry" won't be freed
	     * because the reference count was bumped when it was added to
	     * the IFD.
	     */
	    exif_entry_unref(entry);
	}
	return entry;
}

/* Create a brand-new tag with a data field of the given length, in the
 * given IFD. This is needed when exif_entry_initialize() isn't able to create
 * this type of tag itself, or the default data length it creates isn't the
 * correct length.
 */
static ExifEntry *create_tag(ExifData *exif, ExifIfd ifd, ExifTag tag, size_t len)
{
	void *buf;
	ExifEntry *entry;
	
	/* Create a memory allocator to manage this ExifEntry */
	ExifMem *mem = exif_mem_new_default();
	if (mem == NULL) { /* catch an out of memory condition */
		ESP_LOGI(TAG, "no memo for mem");
		return NULL;
	}

	/* Create a new ExifEntry using our allocator */
	entry = exif_entry_new_mem (mem);
	if (entry == NULL) {
		ESP_LOGI(TAG, "no memo for entry");
		exif_mem_unref(mem);
		return NULL;
	}

	/* Allocate memory to use for holding the tag data */
	buf = exif_mem_alloc(mem, len);
	if (buf == NULL) {
		ESP_LOGI(TAG, "no memo for buf");
		exif_entry_unref(entry);
		exif_mem_unref(mem);
		return NULL;
	}

	/* Fill in the entry */
	entry->data = buf;
	entry->size = len;
	entry->tag = tag;
	entry->components = len;
	entry->format = EXIF_FORMAT_UNDEFINED;

	/* Attach the ExifEntry to an IFD */
	exif_content_add_entry (exif->ifd[ifd], entry);

	/* The ExifMem and ExifEntry are now owned elsewhere */
	exif_mem_unref(mem);
	exif_entry_unref(entry);

	return entry;
}

#define DATA_FORMAT "%Y:%m:%d %T"
#define METADATA_MAX_LEN 400  // actually ~270

bool add_metadata(  int image_jpg_x, 
                    int image_jpg_y, 
                    uint8_t * image_jpg, 
                    int image_jpg_len,
                    uint8_t ** image_jpg_output,
                    int * image_jpg_output_size,
                    struct tm * tm)
{
	unsigned char *exif_data;
	unsigned int exif_data_len;
	ExifEntry *entry;
	char date_time[20];

	int meta_img_max_size = image_jpg_len + METADATA_MAX_LEN;
	uint8_t * meta_img = (uint8_t *) malloc(meta_img_max_size);
	int meta_img_idx = 0;


	if (!meta_img) {
		ESP_LOGE(TAG,  "no memo for meta_img\n");
		return false;
	}

	ExifData *exif = exif_data_new();

	if (!exif) {
		ESP_LOGE(TAG,  "Out of memory\n");
		free(meta_img);
		return false;
	}

	strftime(date_time, sizeof(date_time), DATA_FORMAT, tm);
	ESP_LOGI(TAG, "  DateTime: %s\n  image_jpg_x: %d\n  image_jpg_y: %d", date_time, image_jpg_x, image_jpg_y);   


	/* Set the image options */
	exif_data_set_option(exif, EXIF_DATA_OPTION_FOLLOW_SPECIFICATION);
	exif_data_set_data_type(exif, EXIF_DATA_TYPE_COMPRESSED);
	exif_data_set_byte_order(exif, FILE_BYTE_ORDER);

	/* Create the mandatory EXIF fields with default data */
	exif_data_fix(exif);

	/* All these tags are created with default values by exif_data_fix() */
	/* Change the data to the correct values for this image. */
	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_X_DIMENSION);
	if (!entry) {
		ESP_LOGE(TAG,  "init_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}

	exif_set_long(entry->data, FILE_BYTE_ORDER, image_jpg_x);

	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_PIXEL_Y_DIMENSION);
	if (!entry) {
		ESP_LOGE(TAG,  "init_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}

	exif_set_long(entry->data, FILE_BYTE_ORDER, image_jpg_y);

	entry = init_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_COLOR_SPACE);
	if (!entry) {
		ESP_LOGE(TAG,  "init_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}

	exif_set_short(entry->data, FILE_BYTE_ORDER, 1);

	/* Create a EXIF_TAG_USER_COMMENT tag. This one must be handled
	 * differently because that tag isn't automatically created and
	 * allocated by exif_data_fix(), nor can it be created using
	 * exif_entry_initialize() so it must be explicitly allocated here.
	 */
	entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_USER_COMMENT, 
			sizeof(ASCII_COMMENT) + sizeof(FILE_COMMENT) - 2);
	if (!entry) {
		ESP_LOGE(TAG,  "create_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}
	/* Write the special header needed for a comment tag */
	memcpy(entry->data, ASCII_COMMENT, sizeof(ASCII_COMMENT)-1);
	/* Write the actual comment text, without the trailing NUL character */
	memcpy(entry->data+8, FILE_COMMENT, sizeof(FILE_COMMENT)-1);
	/* create_tag() happens to set the format and components correctly for
	 * EXIF_TAG_USER_COMMENT, so there is nothing more to do. */

	/* Create a EXIF_TAG_SUBJECT_AREA tag */
	entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_SUBJECT_AREA,
			   4 * exif_format_get_size(EXIF_FORMAT_SHORT));
	if (!entry) {
		ESP_LOGE(TAG,  "create_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}

	entry->format = EXIF_FORMAT_SHORT;
	entry->components = 4;
	exif_set_short(entry->data, FILE_BYTE_ORDER, image_jpg_x / 2);
	exif_set_short(entry->data+2, FILE_BYTE_ORDER, image_jpg_y / 2);
	exif_set_short(entry->data+4, FILE_BYTE_ORDER, image_jpg_x);
	exif_set_short(entry->data+6, FILE_BYTE_ORDER, image_jpg_y);

	// EXIF_TAG_DATE_TIME_ORIGINAL
	//#define DATE_TIME_NOW "2025:03:18 09:01:02"
	// char date_time_now[20];
	// snprintf(date_time_now, sizeof(date_time_now), "%s", DATE_TIME_NOW);
	// *entry_2;
	entry = create_tag(exif, EXIF_IFD_EXIF, EXIF_TAG_DATE_TIME_ORIGINAL, sizeof(date_time));
	if (!entry) {
		ESP_LOGE(TAG,  "create_tag fail, line: %d\n", __LINE__);
		free(meta_img);
		return false;
	}

	entry->format = EXIF_FORMAT_ASCII;
	entry->components = sizeof(date_time);
	memcpy(entry->data, date_time, sizeof(date_time));

	/* Get a pointer to the EXIF data block we just created */
	exif_data_save_data(exif, &exif_data, &exif_data_len);
	if (exif_data == NULL) {
		exif_data_unref(exif);
		free(meta_img);
		return false;
	}

	/* Write EXIF header */
	memcpy(meta_img, exif_header, exif_header_len);
	meta_img_idx += exif_header_len;

	/* Write EXIF block length in big-endian order */
	meta_img[meta_img_idx++] = (exif_data_len+2) >> 8;
	meta_img[meta_img_idx++] = (exif_data_len+2) & 0xff;

	/* Write EXIF data block */
	memcpy(&meta_img[meta_img_idx], exif_data, exif_data_len);
	meta_img_idx += exif_data_len;

	//ESP_LOGI(TAG, "offset for actual image data: %d\n", meta_img_idx);

	// Make sure the jpeg fits in the buffer
	if (image_data_len + meta_img_idx >= meta_img_max_size) {
		ESP_LOGI(TAG, "jpeg does not fit in image with metadata buffer");
		free(exif_data);
		exif_data_unref(exif);
		free(meta_img);
		return false;		
	}
	/* Write JPEG image data, skipping the non-EXIF header */
	memcpy(&meta_img[meta_img_idx], image_jpg+image_data_offset, image_data_len);
	meta_img_idx += image_data_len;
	
	/* The allocator we're using for ExifData is the standard one, so use
	 * it directly to free this pointer.
	 */
	free(exif_data);
	exif_data_unref(exif);

	*image_jpg_output = meta_img;
	*image_jpg_output_size = meta_img_idx;

	return true;
}