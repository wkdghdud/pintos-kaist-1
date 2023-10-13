/* vm.c: Generic interface for virtual memory objects. */
#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/*--------project3 추가 헤더----------*/
#include "include/threads/vaddr.h"
#include "include/threads/mmu.h"
struct list frame_table;
bool page_less (const struct hash_elem *a_, const struct hash_elem *b_, void *aux UNUSED);
unsigned page_hash (const struct hash_elem *p_, void *aux UNUSED);
/* Initializes the virtual memory subsystem by invoking each subsystem's
 * intialize codes. */
void
vm_init (void) {
	vm_anon_init ();
	vm_file_init ();
#ifdef EFILESYS  /* For project 4 */
	pagecache_init ();
#endif
	register_inspect_intr ();
	list_init(&frame_table);
	/* DO NOT MODIFY UPPER LINES. */
	/* TODO: Your code goes here. */
}

/* Get the type of the page. This function is useful if you want to know the
 * type of the page after it will be initialized.
 * This function is fully implemented now. */
enum vm_type
page_get_type (struct page *page) {
	int ty = VM_TYPE (page->operations->type);
	switch (ty) {
		case VM_UNINIT:
			return VM_TYPE (page->uninit.type);
		default:
			return ty;
	}
}

/* Helpers */
static struct frame *vm_get_victim (void);
static bool vm_do_claim_page (struct page *page);
static struct frame *vm_evict_frame (void);
// static struct list frame_table;
static struct list_elem *start;//frame_table 순회하기 위한 첫번째 원소

/* Create the pending page object with initializer. If you want to create a
 * page, do not create it directly and make it through this function or
 * `vm_alloc_page`. */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)

	struct supplemental_page_table *spt = &thread_current ()->spt;
	/* upage가 이미 점유되었는지 여부를 확인합니다. */
	if (spt_find_page (spt, upage) == NULL) {//uspage라는 가상 메모리에 매핑되는 페이지가 존재하지 않기 떄문에 새로만들어야함
	/* TODO: 페이지를 생성하고 VM 유형에 따라 초기화를 가져옵니다.
	   TODO: 그런 다음 "uninit" 페이지 구조체를 uninit_new를 호출하여 생성하십시오.
	   TODO: uninit_new를 호출한 후에 필드를 수정해야 합니다. */
	   struct page *page = (struct page*)malloc(sizeof(struct page));
	   //페이지 유형에 따라  초기화 함수를 매칭해준다.
	   //일단 spt에 upage 주소가 없음 uppage는 새롭게 만들 페이지, 프러세스c에서 logsed에서 호출함 페이지 하나 할당
	   void *rd_page = pg_round_down(upage);//정렬할라고 ㅋㅋ
	   switch (type)
	   {
	   case VM_UNINIT:
		/* code */
		break;
	   case VM_ANON :
		/* code */
		uninit_new(page,rd_page,init,type,aux,anon_initializer);
		break;
		case VM_FILE:
		/* code */
		break;
	   case VM_PAGE_CACHE:
		/* code */
		break;		   
	   default:
		break;
	   }
	// uninit_new(page,rd_page,init,type,aux,anon_initializer);
	   if (!spt_insert_page(spt,page)){
		printf("%p\n",page->va);
		return false;
	   }
	   return true;
	/* TODO: 페이지를 SPT에 삽입하십시오. */
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	/*va를 통해서 page를 찾아야 함, hash_find의 인자는 hash_elem이므로 이에 해당하는 hash_elem을 만듬*/
	struct page *page;
	page = (struct page *)malloc(sizeof(struct page));//페이지를 새로 만들어주는 이유->해당 가상 주소에 대응하는 해시값 도출,page_elem도 같이 생성됨
	/* TODO: Fill this function. */
	page->va = pg_round_down(va);//페이지 매핑 작업
	struct hash_elem *e;
	/*e와 같은 해시값을 갖는 page를 spt에서 찾은다음 해당 hash_elem을 리턴*/
	e = hash_find(&spt->spt_hash,&page->hash_elem);
	free(page);
	if(e == NULL)
		return NULL;

	return hash_entry(e, struct page, hash_elem);
}

/* Insert PAGE into spt with validation. */
bool
spt_insert_page (struct supplemental_page_table *spt UNUSED,
		struct page *page UNUSED) {
	//int succ = false;
	/* TODO: Fill this function. */
	//return succ;
	return hash_insert(&spt->spt_hash,&page->hash_elem)==NULL?true:false;
}

void
spt_remove_page (struct supplemental_page_table *spt, struct page *page) {
	vm_dealloc_page (page);
	return true;
}

/* Get the struct frame, that will be evicted. */
static struct frame *
vm_get_victim (void) {
	struct frame *victim = NULL;
	 /* TODO: The policy for eviction is up to you. */

	return victim;
}

/* Evict one page and return the corresponding frame.
 * Return NULL on error.*/
static struct frame *
vm_evict_frame (void) {
	struct frame *victim UNUSED = vm_get_victim ();
	/* TODO: swap out the victim and return the evicted frame. */

	return NULL;
}

/* palloc() and get frame. If there is no available page, evict the page
 * and return it. This always return valid address. That is, if the user pool
 * memory is full, this function evicts the frame to get the available memory
 * space.*/
static struct frame *
vm_get_frame (void) {
	struct frame *frame = (struct frame*)malloc(sizeof(struct frame));
	//frame 구조체를 위한 공간 할당, 작으므로 Malloc으로 깃북 memory allocation에 나옴
	//커널이랑 매칭될 놈이기 떄문에 ㅋㅋ
	/* TODO: Fill this function. */
	/*palloc_get_page : 사용 가능한 단일 페이지를 가져오고 커널 가상주소를 반환한다. PAL_USER가 설정되어 있으면
	사용자 풀에서 페이지를 가져오고, 그렇지 않으먼 커널풀에서 페이지를 가져온다. FLAGS에 PAL_ZERO가 설정되어있으면
	그 페이지는 0으로 채워진다. 사용가능한 페이지가 없으면 FLAGS에서 PAL_ASSERT가 설정되어 있지 않은 경우 NULL포인터를
	반환한다. 이 경우 커널 패닉이 발생*/
	frame -> kva = palloc_get_page(PAL_USER|PAL_ZERO);//유저풀(실제메모리)로 부터 페이지 하나를 가져온다, 사용가능한 페이지가 없을 경우 null리턴
	if(frame->kva==NULL || frame == NULL){
		// frame = vm_evict_frame();//swap out 수행(프레임을 내쫓고 해당 공간을 가져온다.)
		// frame->page=NULL;
		// return frame;//무조건 유효한 공간만 리턴한다 -> swap out을 통해 공간확보후 리턴하기 떄문
		PANIC("TODO");
	}
	

	list_push_back(&frame_table , &frame->frame_elem);
	frame->page=NULL;//페이지랑 매칭이 안됐으니까
	ASSERT (frame != NULL);
	ASSERT (frame->page == NULL);
	return frame;
}

/* Growing the stack. */
static void
vm_stack_growth (void *addr UNUSED) {
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

/* Return true on success */
bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt = &thread_current ()->spt;
	struct page *page = NULL;
	/* TODO: Validate the fault */
	/* TODO: Your code goes here */
	/*spt addr로 페이지를 찾는다
	페이지가 없으면 펄스 페이지가 있으면 두클레임 페이지를 한다
	swap_in이 성공하면 트루*/
	page = spt_find_page(spt, pg_round_down(addr));
	


	if(page ==NULL){
		return false;
	}

	return vm_do_claim_page (page);
}

/* Free the page.
 * DO NOT MODIFY THIS FUNCTION. */
void
vm_dealloc_page (struct page *page) {
	destroy (page);
	free (page);
}

/* Claim the page that allocate on VA. */
bool
vm_claim_page (void *va UNUSED) {
	//struct page *page = NULL;
	/* TODO: Fill this function */
	struct thread *curr = thread_current();
	struct page *page = spt_find_page(&curr -> spt , va);
	if(page == NULL)
		return false;
	return vm_do_claim_page (page);
}

/* Claim the PAGE and set up the mmu. */
static bool
vm_do_claim_page (struct page *page) {
	struct frame *frame = vm_get_frame ();
	
  /* Set links */
	frame->page = page; // 프레임의 페이지(가상)로 얻은 페이지를 연결해준다.
	page->frame = frame; // 페이지의 물리적 주소로 얻은 프레임을 연결해준다.
	/* TODO: Insert page table entry to map page's VA to frame's PA. */
	struct thread *curr = thread_current();
	// bool writable = page -> writable; // 해당 페이지의 R/W 여부
	// pml4_set_page(curr->pml4, page->va, frame->kva, writable); // 현재 페이지 테이블에 가상 주소에 따른 frame 매핑

	if(!pml4_set_page(curr->pml4, page->va, frame->kva,true))
		return false;
	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init (&spt->spt_hash, page_hash, page_less, NULL);
}

/* Copy supplemental page table from src to dst */
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,
		struct supplemental_page_table *src UNUSED) {
}

/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
}
unsigned
page_hash (const struct hash_elem *p_, void *aux UNUSED) {
  const struct page *p = hash_entry (p_, struct page, hash_elem);
  return hash_bytes (&p->va, sizeof (p->va));
}

bool
page_less (const struct hash_elem *a_,
           const struct hash_elem *b_, void *aux UNUSED) {
  const struct page *a = hash_entry (a_, struct page, hash_elem);
  const struct page *b = hash_entry (b_, struct page, hash_elem);

  return a->va < b->va;
}