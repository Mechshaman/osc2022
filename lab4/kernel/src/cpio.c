#include "cpio.h"
#include "uart.h"
#include "string.h"
#include "utils.h"
#include "exec.h"

/* Parse an ASCII hex string into an integer. */
static unsigned long parse_hex_str(char *s, unsigned int max_len)
{
    unsigned long r = 0;
    unsigned long i;

    for (i = 0; i < max_len; i++) {
        r *= 16;
        if (s[i] >= '0' && s[i] <= '9') {
            r += s[i] - '0';
        }  else if (s[i] >= 'a' && s[i] <= 'f') {
            r += s[i] - 'a' + 10;
        }  else if (s[i] >= 'A' && s[i] <= 'F') {
            r += s[i] - 'A' + 10;
        } else {
            return r;
        }
        continue;
    }
    return r;
}

/*
 * Parse the header of the given CPIO entry.
 *
 * Return -1 if the header is not valid, 1 if it is EOF.
 */
int cpio_parse_header(struct cpio_header *archive,
        const char **filename, unsigned long *_filesize, void **data,
        struct cpio_header **next)
{
    unsigned long filesize;
    /* Ensure magic header exists. */
    if (strncmp(archive->c_magic, CPIO_HEADER_MAGIC,
                sizeof(archive->c_magic)) == 0)
        return -1;

    /* Get filename and file size. */
    filesize = parse_hex_str(archive->c_filesize, sizeof(archive->c_filesize));
    *filename = ((char *)archive) + sizeof(struct cpio_header);

    /* Ensure filename is not the trailer indicating EOF. */
    if (strncmp(*filename, CPIO_FOOTER_MAGIC, sizeof(CPIO_FOOTER_MAGIC)))
        return 1;

    /* Find offset to data. */
    unsigned long filename_length = parse_hex_str(archive->c_namesize,
            sizeof(archive->c_namesize));
    *data = (void *)align_up(((unsigned long)archive)
            + sizeof(struct cpio_header) + filename_length, CPIO_ALIGNMENT);
    *next = (struct cpio_header *)align_up(((unsigned long)*data) + filesize, CPIO_ALIGNMENT);
    if(_filesize){
        *_filesize = filesize;
    }
    return 0;
}

void ls() {
    void *archive = cpio_addr;
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long size;

    header = archive;
    while(header!=0) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;
        printf("%s\r\n",current_filename);

        header = next;
    }
}

void cat(char* thefilepath) {
    void *archive = cpio_addr;
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long size;

    header = archive;
    while(header!=0) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;

        if(strcmp(thefilepath,current_filename))
        {
            uart_puts_n(result,size);
            uart_puts("\r\n");
            break;
        }

        header = next;
    }

}

void execfile(char* thefilepath) {
    void *archive = cpio_addr;
    const char *current_filename;
    struct cpio_header *header, *next;
    void *result;
    int error;
    unsigned long size;

    header = archive;
    while(header!=0) {
        error = cpio_parse_header(header, &current_filename, &size,
                &result, &next);
        // Break on an error or nothing left to read.
        if (error) break;

        if(strcmp(thefilepath,current_filename))
        {
            exec(result);
            break;
        }

        header = next;
    }

}