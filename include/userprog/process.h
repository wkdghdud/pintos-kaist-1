#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"


/* Max file descriptor */
#define MAX_FD 500
#define MIN_FD 4
#define STDIN_FILENO 1
#define STDOUT_FILENO 2


tid_t process_create_initd (const char *file_name);
tid_t process_fork (const char *name, struct intr_frame *if_);
int process_exec (void *f_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (struct thread *next);

bool
lazy_load_segment_for_file(struct page *page, void *aux);

struct child_info *get_child_info(struct thread *curr, int child_pid);


/* Project3 - Anon Page */
/*해당 구조체는 디스크에 저장되어 있지 않기 때문에, 특정 파일을 불러 오는 것이 아님 
우리는 현재 lazy loading 방식을 취하고 있고 이는 파일 전체를 다 읽어오지 않는다. 
그때 그때 필요할 때만 읽어오는데, 그걸 위해서는 우리가 어떤 파일의 어떤 
위치에서 읽어와야 할지 알아야 하고 그 정보가 container안에 들어가 있다
*/
struct lazy_mmap
{
	struct page *head_page;
	void *addr;
	void *upage;
	size_t length;
	size_t page_read_bytes;
	int writable;
	struct file *file;
	off_t offset;
	
};

struct segment_aux{
	struct file *file;
	off_t ofs;
	uint8_t *upage;
	uint32_t read_bytes;
	uint32_t zero_bytes;
	bool writable;
};

struct lazy_load_info
{
    struct file *file;
    size_t page_read_bytes;
    size_t page_zero_bytes;
    off_t offset
};



#endif /* userprog/process.h */
