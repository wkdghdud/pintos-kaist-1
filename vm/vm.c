/* vm.c: Generic interface for virtual memory objects. */
#include "threads/malloc.h"
#include "vm/vm.h"
#include "vm/inspect.h"

/*--------project3 추가 헤더----------*/
#include "include/threads/vaddr.h"
#include "include/threads/mmu.h"
#include "include/threads/thread.h"
#include "include/userprog/process.h"

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

/* "초기화자와 함께 보류 중인 페이지 객체를 생성합니다.
 페이지를 생성하려면 직접 만들지 말고 이 함수나 vm_alloc_page를 통해 만드세요." */
bool
vm_alloc_page_with_initializer (enum vm_type type, void *upage, bool writable,
		vm_initializer *init, void *aux) {

	ASSERT (VM_TYPE(type) != VM_UNINIT)
	uint8_t *rd_page = pg_round_down(upage);
	struct supplemental_page_table *spt = &thread_current ()->spt;
	/* upage가 이미 점유되었는지 여부를 확인합니다. */
	if (spt_find_page (spt, rd_page) == NULL) {//uspage라는 가상 메모리에 매핑되는 페이지가 존재하지 않기 떄문에 새로만들어야함
	/* TODO: 페이지를 생성하고 VM 유형에 따라 초기화를 가져옵니다.
	   TODO: 그런 다음 "uninit" 페이지 구조체를 uninit_new를 호출하여 생성하십시오.
	   TODO: uninit_new를 호출한 후에 필드를 수정해야 합니다. */
	   struct page *page = (struct page*)calloc(1,sizeof(struct page));
	   //페이지 유형에 따라  초기화 함수를 매칭해준다.
	   //일단 spt에 upage 주소가 없음 uppage는 새롭게 만들 페이지, 프러세스c에서 logsed에서 호출함 페이지 하나 할당
	   //void *rd_page = pg_round_down(upage);//정렬할라고
	   //uint8_t *rd_page = pg_round_down(upage);
		if (page == NULL)
			goto err;
		switch (type)
		{
		case VM_UNINIT:
			/* code */
			uninit_new(page, rd_page, init, type, aux, NULL);
			page->writable = writable;
			break;
		case VM_ANON:
			/* code */
			uninit_new(page, rd_page, init, type, aux, anon_initializer);
			page->writable = writable;
			break;
		case VM_FILE:
			/* code */
			uninit_new(page, rd_page, init, type, aux, file_backed_initializer);
			page->writable = writable;
			break;
		case VM_PAGE_CACHE:
			/* code */
			uninit_new(page, rd_page, init, type, aux, NULL);
			page->writable = writable;
			break;
		default:
			break;
		}
	// uninit_new(page,rd_page,init,type,aux,anon_initializer);
	   if (!spt_insert_page(spt,page)){
		printf("%p\n",page->va);
		return false;
	   }
	   hash_insert(&spt->spt_hash, &page->hash_elem);
	   return true;
	}
err:
	return false;
}

/* Find VA from spt and return page. On error, return NULL. */
struct page *
spt_find_page (struct supplemental_page_table *spt UNUSED, void *va UNUSED) {
	
	/*va를 통해서 page를 찾아야 함, hash_find의 인자는 hash_elem이므로 이에 해당하는 hash_elem을 만듬*/
	struct page *page=NULL;
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
	int succ = false;
	if(spt_find_page(spt, page->va)== NULL){
		hash_insert(&spt->spt_hash, &page->hash_elem);
		succ = true;
	}
	return succ;
	//int succ = false;
	/* TODO: Fill this function. */
	//return succ;
	//return hash_insert(&spt->spt_hash,&page->hash_elem)==NULL?true:false;
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
	vm_alloc_page(VM_ANON,addr,true);
}

/* Handle the fault on write_protected page */
static bool
vm_handle_wp (struct page *page UNUSED) {
}

// /* Return true on success */
// bool
// vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
// 		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
// 	struct supplemental_page_table *spt = &thread_current ()->spt;
// 	struct page *page = NULL;
// 	/* TODO: Validate the fault */
// 	/* TODO: Your code goes here */
//     /* page_fault로 부터 넘어온 인자
//      * f : 시스템 콜 또는 페이지 폴트가 발생했을 때,그 순간의 레지스터 값들을 담고 있는 구조체.
//      * addr : 페이지 폴트를 일으킨 가상 주소
//      * user : 유저에 의한 접근(true), 커널에 의한 접근(false) - rsp 값이 유저 영역인지 커널영역인지
//      * write : 쓰기 목적 접근(true), 읽기 목적 접근(false)
//      * not_present : 페이지 존재 x (bogus fault), false인 경우 read-only페이지에 write하려는 상황
//     */
//    	/*
// 	1.인자로 들어오면 addr의 유효성을 검증 -ㄲㅈ
// 	2.현재 쓰레드에 rsp_stack을 받아오거나 인터럽트 프레임의 rsp를 받아와 현재 쓰레드의 rsp주소를 설정-가지마
// 	3.이후 rsp가 유효하면 실제 vm_stack_growth()를 호출한다.
// 	4.그다음 해당 주소에 페이지를 만들었으니, spt_find로 해당 페이지를 찾은 뒤 해당 페이지의 구조체와 write인자를 통해 쓰기 검사를 진행
// 	5.이후 해당 페이지에 대해 vm_do_claim_page()를 진행하여 실제로 물리프레임과 해당 페이지를 연결시켜줌,uninit->anon
// 	6.위 과정을 거친다면 true를 반환하며 함수를 종료
// 	thread.h 파일에 void* rsp_stack과 void* stack_bottom;, rsp_stack의 경우 현재 쓰레드의 rsp 주소값을 담는 변수이고
// 	stack_bottom은 현재 쓰렏의 stack영역의 끝 지점을 파악하기 위해 선언하는 변수, 즉, stack을 벗어나느 접근이 있을 경우 예외처리하기 위해사용
// 	*/
//     if (is_kernel_vaddr(addr) || addr == NULL) {
//         return false;
//     }

//     // 2. 주소(addr)를 페이지 경계로 정렬
//     void *aligned_addr = pg_round_down(addr);
    
//     page = spt_find_page(spt, aligned_addr);
    
//     if (page == NULL) {
//         // 페이지가 찾을 수 없는 경우, 유저 스택 영역인지 확인하고 스택을 확장
//         if (aligned_addr >= USER_STACK - PGSIZE && aligned_addr >= USER_STACK - (1 << 20)) {
//             vm_stack_growth(aligned_addr);
//             page = spt_find_page(spt, aligned_addr);
//         } else {
//             return false; // 페이지를 찾을 수 없으면 처리 거부
//         }
//     }

//     // 4. 쓰기 권한을 확인하고 페이지를 처리
//     if (write && !page->writable) {
//         return false; // 쓰기 권한이 필요하면서 페이지가 읽기 전용인 경우 처리 거부
//     }

//     // 6. 페이지를 처리하고 성공 여부를 반환
//     return vm_do_claim_page(page);
// }


bool
vm_try_handle_fault (struct intr_frame *f UNUSED, void *addr UNUSED,
		bool user UNUSED, bool write UNUSED, bool not_present UNUSED) {
	struct supplemental_page_table *spt UNUSED = &thread_current ()->spt;
	struct page *page = NULL;

	if(is_kernel_vaddr(addr) || addr==NULL){
		return false;
	}
	void* rd_addr = pg_round_down(addr);
	page = spt_find_page(spt,rd_addr);
	if(page ==NULL){
		if(rd_addr > USER_STACK-(1<<20) && rd_addr < USER_STACK&& addr == f->rsp){
			vm_stack_growth(rd_addr);
			page = spt_find_page(spt,rd_addr);
		}
		else
			return false;
	}
	//ro pass
	else{
		if(page->writable == false && write == true)
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

	if(!pml4_set_page(curr->pml4, page->va, frame->kva,page->writable))
		return false;
	return swap_in (page, frame->kva);
}

/* Initialize new supplemental page table */
void
supplemental_page_table_init (struct supplemental_page_table *spt UNUSED) {
	hash_init (&spt->spt_hash, page_hash, page_less, NULL);
}
	/*
		1.해시테이블을 통해 구현한 spt를 iteration(반복)해줘야함
		2.while문을 통해 iter해주는 방식
		3.이후 iter를 돌때마다 해당 hash_elem과 연결된 page를 찾아서 해당 페이지 구조체의 정보를 저장
		4.페이지 구조체의 어떠한 정보를 저장해야할지는 vm_alloc_page_with_initializer()함수 인자 참고
		5.부모 페이지들의 정보를 저장한뒤, 자식이 가질 새로운 페이지를 생성, 생성을 위해 부모 페이지의 타입을 검사
		즉, 부모 페이지가 unnit페이지인 경우 그렇지 않은 경우를 나누어 페이지를 생성
		6.만약 unnit이 아닌 경우 setup_stack()함수에서 했던 것처럼 페이지를 생성한 뒤 바로 해당 페이지의 타입에 맞는
		initializer를 호출해 타입을 변경시켜줍니다. 그리고 나서 부모 페이지의 물리 메모리 정보를 자식에게도 복사해주어야 합니다.
		7.모든 함수 호출이 정상적으로 이루어 졌다면 return true
	*/
/* "src로부터 dst로 보조 페이지 테이블을 복사합니다."*/
bool
supplemental_page_table_copy (struct supplemental_page_table *dst UNUSED,struct supplemental_page_table *src UNUSED) {
	
	struct hash_iterator i;//해시테이블을 이터레이팅 하기 위한 이터레이터 i를 정의
	hash_first (&i, &src->spt_hash);//hash_first 함수를 호출하여 i 이터레이터를 해시테이블의 첫번째 요소로 초기화함

	while (hash_next (&i)){//src 복사
		struct page *parent_page = hash_entry (hash_cur (&i), struct page, hash_elem);

		//type,upage,writable,*init,*aux 부모 페이지에 이러한 정보 저장해야됨
		enum vm_type type = parent_page->operations->type;
		void *upage = parent_page->va;
		bool writable = parent_page->writable;
	// vm_initializer *init = parent_page->uninit.init;
	// void *aux = parent_page->uninit.aux;

		if(type== VM_UNINIT){ //초기화 되지 않은 페이지를 나타냄
	    	vm_initializer *init = parent_page->uninit.init;
	    	void *aux = parent_page->uninit.aux;
			vm_alloc_page_with_initializer(VM_ANON,upage,writable,init,aux);//부모 페이지의 타입, 가상주소, 쓰기가능여부, 초기화 함수, 초기화 함수에 대한 보조정보를 초기화함, 이걸 자식에게 넘기면 자식페이지는 부모페이지와 동일한 초기 상태가 됨
				continue;
			}
		if(!vm_alloc_page(type, upage,writable)){ // 함수를 사용하여 자식페이지를 생성
			return false;
			}

		if(!vm_claim_page(upage)){// 자식 페이지를 물리 메모리에 할당
			return false;
			}
		struct page *child_page = spt_find_page(dst,upage);//dst 보조테이블에서 현재 복사할 가상주소 upage에 해당하는 자식페이지 찾음
		memcpy(child_page->frame->kva,parent_page->frame->kva,PGSIZE);//자식 페이지 프레임,부모 페이지 프레임 주소 4kb만큼 복사
	}
	return true;
}

void hash_page_destory(struct hash_elem *e, void *aux)
{
	struct page *page = hash_entry(e, struct page, hash_elem);
	if(page_get_type(page) == VM_FILE){
		if(page->file.file != NULL){
			file_seek(page->file.file,0);
			file_write(page->file.file, page->frame->kva,file_length(page->file.addr));
		}
	}
	//vm_dealloc_page(page);
	destroy(page);
	free(page);
}


/* Free the resource hold by the supplemental page table */
void
supplemental_page_table_kill (struct supplemental_page_table *spt UNUSED) {
	/* TODO: Destroy all the supplemental_page_table hold by thread and
	 * TODO: writeback all the modified contents to the storage. */
	// printf("Dddddddddd\n");
	hash_clear(&spt->spt_hash,hash_page_destory);
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