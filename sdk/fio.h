#ifndef __FIO_H__
#define __FIO_H__

#include <util/types.h>
#include <util/fifo.h>

#define FIO_INPUT_BUFFER_SIZE	1024
#define FIO_OUTPUT_BUFFER_SIZE	1024
#define FIO_MAX_REQUEST_ID	256
#define FIO_MAX_NAME_LEN	256

#define FIO_EOF			0
#define FIO_OK			1
#define FIO_ERR_NOSPC		4	/* No space left on device */
#define FIO_ERR_IO		5      	/* I/O error */
#define FIO_ERR_OPENMODE	7	/* Invalid open mode */
#define FIO_ERR_FILENOTEXIST	8	/* File not exist */
#define FIO_ERR_BADFD		9	/* Bad file descriptor */
#define FIO_ERR_BADBUF		10	/* Bad buffer address */
#define FIO_ERR_READONLY	11	/* Read only */
#define FIO_ERR_BADSIZE		12	/* Negative size */
#define FIO_ERR_BUSY		29	/* I/O Busy */
#define FIO_ERR_FTYPE		79	/* Inappropriate file type */
#define FIO_ERR_FIFOBUSY	90	/* Fifo is in use */
#define FIO_ERR_NOBUFS		105	/* No buffer space available */
#define FIO_ERR_FSDRIVER	106	/* Bad File system driver */

/**
 * Directory data structure
 */
typedef struct {
	char			name[FIO_MAX_NAME_LEN];
} Dirent;

typedef struct {
	// Buffer
	FIFO*			input_buffer;		///< Input buffer
	FIFO*			output_buffer;		///< Output buffer

	// Lock
	uint8_t			input_lock;		///< Input buffer lock
	uint8_t			output_lock;		///< Input buffer lock

	// Request ID
	uint32_t		request_id;		///< Request id

	// Event ID
	uint64_t		event_id;		///< Polling event id
} FIO;

/**
 * I/O request data structure
 */
typedef struct {
#define	FILE_T_OPEN		1			/* Type of the request for open */
#define	FILE_T_CLOSE		2			/* Type of the request for close */
#define	FILE_T_READ		3			/* Type of the request for read */
#define	FILE_T_WRITE		4			/* Type of the request for write */
#define	FILE_T_OPENDIR		5			/* Type of the request for open dir */
#define	FILE_T_CLOSEDIR		6			/* Type of the request for close dir */
#define	FILE_T_READDIR		7			/* Type of the request for read dir */
	uint32_t		id;			///< Request id
	uint32_t		type;			///< Type of the request
	int			fd;			///< File descriptor
	void*			context;		///< User context

	union {
		struct {
			char	name[FIO_MAX_NAME_LEN];	///< Name of file or directory
			char	flags[10];		///< Flags for open
		} open;
		
		struct {
			void*	buffer;			///< Buffer that user hands over
			size_t	size;			///< Size that user wants to read or write
		} file_io;
		
		struct {
			Dirent*	dir;			///< Direct that user hands over
		} read_dir;
	} op;
} FIORequest;

FIO* fio_create(void* pool);

#endif /* __FIO_H__ */
