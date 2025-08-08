#include <errno.h>
#include <fcntl.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/queue.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>

typedef uint32_t u32;
typedef int32_t i32;

struct process
{
  u32 pid;
  u32 arrival_time;
  u32 burst_time;

  TAILQ_ENTRY(process) pointers;

  // Additional fields here
  u32 remaining_time;
  i32 first_run_time;
  //u32 finish_time;
  /* End of "Additional fields here" */
};

TAILQ_HEAD(process_list, process);

u32 next_int(const char **data, const char *data_end)
{
  u32 current = 0;
  bool started = false;
  while (*data != data_end)
  {
    char c = **data;

    if (c < 0x30 || c > 0x39)
    {
      if (started)
      {
        return current;
      }
    }
    else
    {
      if (!started)
      {
        current = (c - 0x30);
        started = true;
      }
      else
      {
        current *= 10;
        current += (c - 0x30);
      }
    }

    ++(*data);
  }

  printf("Reached end of file while looking for another integer\n");
  exit(EINVAL);
}

u32 next_int_from_c_str(const char *data)
{
  char c;
  u32 i = 0;
  u32 current = 0;
  bool started = false;
  while ((c = data[i++]))
  {
    if (c < 0x30 || c > 0x39)
    {
      exit(EINVAL);
    }
    if (!started)
    {
      current = (c - 0x30);
      started = true;
    }
    else
    {
      current *= 10;
      current += (c - 0x30);
    }
  }
  return current;
}

void init_processes(const char *path,
                    struct process **process_data,
                    u32 *process_size)
{
  int fd = open(path, O_RDONLY);
  if (fd == -1)
  {
    int err = errno;
    perror("open");
    exit(err);
  }

  struct stat st;
  if (fstat(fd, &st) == -1)
  {
    int err = errno;
    perror("stat");
    exit(err);
  }

  u32 size = st.st_size;
  const char *data_start = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (data_start == MAP_FAILED)
  {
    int err = errno;
    perror("mmap");
    exit(err);
  }

  const char *data_end = data_start + size;
  const char *data = data_start;

  *process_size = next_int(&data, data_end);

  *process_data = calloc(sizeof(struct process), *process_size);
  if (*process_data == NULL)
  {
    int err = errno;
    perror("calloc");
    exit(err);
  }

  for (u32 i = 0; i < *process_size; ++i)
  {
    (*process_data)[i].pid = next_int(&data, data_end);
    (*process_data)[i].arrival_time = next_int(&data, data_end);
    (*process_data)[i].burst_time = next_int(&data, data_end);
  }

  munmap((void *)data, size);
  close(fd);
}

int main(int argc, char *argv[])
{
  if (argc != 3)
  {
    return EINVAL;
  }
  struct process *data;
  u32 size;
  init_processes(argv[1], &data, &size);

  u32 quantum_length = next_int_from_c_str(argv[2]);

  struct process_list list;
  TAILQ_INIT(&list);

  u32 total_waiting_time = 0;
  u32 total_response_time = 0;

  /* Your code here */
  for(u32 i = 0; i < size; ++i){
    data[i].remaining_time = data[i].burst_time;
    data[i].first_run_time = -1;
    TAILQ_INSERT_TAIL(&list, &data[i], pointers);
  }

  struct process_list ready_queue;
  TAILQ_INIT(&ready_queue);

  u32 current_time = 0;
  u32 completed = 0;
  

  struct process_list pending;
  TAILQ_INIT(&pending);
  for(u32 i = 0; i < size; ++i){
    TAILQ_INSERT_TAIL(&pending, &data[i], pointers);
  }
  while(completed < size){
    struct process *p, *tmp;
    p = TAILQ_FIRST(&list);
    while(p != NULL){
     tmp = TAILQ_NEXT(p, pointers);
      if(p->arrival_time <= current_time){
        TAILQ_REMOVE(&list, p, pointers);
        TAILQ_INSERT_TAIL(&ready_queue, p, pointers);
      }
      p = tmp;
    }

    if(TAILQ_EMPTY(&ready_queue)){
      current_time++;
      continue;
    }

    p = TAILQ_FIRST(&ready_queue);
    TAILQ_REMOVE(&ready_queue, p, pointers);

    if(p->first_run_time == -1){
      p->first_run_time = current_time;
      total_response_time += (p->first_run_time - p->arrival_time);
    }

    u32 run_for = (p->remaining_time > quantum_length) ? quantum_length : p->remaining_time;
    p->remaining_time -= run_for;
    current_time += run_for;

    struct process *a = TAILQ_FIRST(&pending);
    while(a!= NULL){
      tmp = TAILQ_NEXT(a, pointers);
      if(a->arrival_time <= current_time){
        TAILQ_REMOVE(&pending, a, pointers);
        TAILQ_INSERT_TAIL(&ready_queue, a, pointers);
      }
      a = tmp;
    }
    /*
    struct process *arriving = TAILQ_FIRST(&list);
    while(arriving != NULL){
      struct process *next = TAILQ_NEXT(arriving, pointers);
      if(arriving->arrival_time <= current_time){
        TAILQ_REMOVE(&list, arriving, pointers);
        TAILQ_INSERT_TAIL(&ready_queue, arriving, pointers);
      }
      arriving = next;
    }
    */
    if(p->remaining_time > 0){
   TAILQ_INSERT_TAIL(&ready_queue, p, pointers);
    }
    else{
        //p->finish_time = current_time;
        total_waiting_time+= (current_time- p->arrival_time - p->burst_time);
        completed++;
    }
  }


  /* End of "Your code here" */

  printf("Average waiting time: %.2f\n", (float)total_waiting_time / (float)size);
  printf("Average response time: %.2f\n", (float)total_response_time / (float)size);

  free(data);
  return 0;
}
