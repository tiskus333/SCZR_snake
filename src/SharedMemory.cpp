#include "SharedMemory.hpp"

#include <iostream>

void createSharedMemory(const char *path_name) {
  // create shared memory structure
  int fd = shm_open(path_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("main: Cannot create shared memory.");
    exit(EXIT_FAILURE);
  }

  // shrink creared shared memory to frameBuffer size
  if (ftruncate(fd, sizeof(sh_m)) == -1) {
    perror("main: ftruncate error.");
    exit(EXIT_FAILURE);
  }

  // map frame buffer onto shared memory
  sh_m *shmp = (sh_m *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("main: Cannot map memory.");
    exit(EXIT_FAILURE);
  }

  shmp->size = DATA_SIZE;

  // initialize semaphores as process-shared
  if (sem_init(&shmp->sem_read, 1, 0) == -1) {
    perror("main: Cannot init sem_read.");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&shmp->sem_write, 1, 1) == -1) {
    perror("main: Cannot init sem_write.");
    exit(EXIT_FAILURE);
  }
}

sh_m *openSharedMemory(const char *path_name) {
  // open existing shared memory
  int fd = shm_open(path_name, O_RDWR, 0);
  if (fd == -1) {
    perror("Cannot open shared memory.");
    exit(EXIT_FAILURE);
  }

  sh_m *shmp = (sh_m *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("Cannot map memory.");
    exit(EXIT_FAILURE);
  }
  return shmp;
}

gm_st *createSharedGameState(const char *path_name) {
  // create shared memory structure
  int fd = shm_open(path_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("main: Cannot create shared memory.");
    exit(EXIT_FAILURE);
  }

  // shrink creared shared memory to frameBuffer size
  if (ftruncate(fd, sizeof(gm_st)) == -1) {
    perror("main: ftruncate error.");
    exit(EXIT_FAILURE);
  }

  // map frame buffer onto shared memory
  gm_st *shmp = (gm_st *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("main: Cannot map memory.");
    exit(EXIT_FAILURE);
  }

  shmp->read_count = 0;
  shmp->write_count = 0;

  // initialize semaphores as process-shared
  if (sem_init(&shmp->sem_read, 1, 1) == -1) {
    perror("main: Cannot init sem_read.");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&shmp->sem_write, 1, 1) == -1) {
    perror("main: Cannot init sem_write.");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&shmp->try_read, 1, 1) == -1) {
    perror("main: Cannot init sem_read.");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&shmp->resource, 1, 1) == -1) {
    perror("main: Cannot init sem_write.");
    exit(EXIT_FAILURE);
  }
  return shmp;
}

gm_st *openSharedGameState(const char *path_name) {
  // open existing shared memory
  int fd = shm_open(path_name, O_RDWR, 0);
  if (fd == -1) {
    perror("Cannot open shared memory.");
    exit(EXIT_FAILURE);
  }

  gm_st *shmp = (gm_st *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("Cannot map memory.");
    exit(EXIT_FAILURE);
  }
  return shmp;
}

void SharedFrame::sendToSharedMemory(const unsigned char *p_data,
                                      const size_t data_size) {
  // close write semaphore
  if (sem_wait(&sem_write)) {
    perror("A: sem_wait error.");
    exit(EXIT_FAILURE);
  }
  memcpy(&this->frame, p_data, std::min(this->size, data_size));
  // open read semaphore
  if (sem_post(&sem_read)) {
    perror("A: sem_post error.");
    exit(EXIT_FAILURE);
  }
}

void SharedFrame::receiveFromSharedMemory(unsigned char *p_write_to,
                                           const size_t data_size) {
  // close read semaphore
  if (sem_wait(&sem_read)) {
    perror("A: sem_wait error.");
    exit(EXIT_FAILURE);
  }
  memcpy(p_write_to, &this->frame, std::min(this->size, data_size));
  // open write semaphore
  if (sem_post(&sem_write)) {
    perror("A: sem_post error.");
    exit(EXIT_FAILURE);
  }
}
void SharedFrame::readFrame(const cv::Mat &frame) {
  // receiving data from shared memory
  receiveFromSharedMemory(frame.data, DATA_SIZE);
}

void SharedFrame::writeFrame(const cv::Mat &frame) {
  // writing data into shared memory
  sendToSharedMemory(frame.data, DATA_SIZE);
}

char SharedGameState::readKey() {
  char key;
  if (sem_wait(&try_read)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&sem_read)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  ++read_count;
  if(read_count == 1) {
    if (sem_wait(&resource)) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
  }
  if(sem_post(&sem_read)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
  if(sem_post(&try_read)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }

  memcpy(&key, &this->key_pressed, sizeof(char));

  if (sem_wait(&sem_read)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  --read_count;
  if(read_count == 0) {
    if(sem_post(&resource)) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
  }
  if(sem_post(&sem_read)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
  return key;
}

void SharedGameState::writeKey(const char &key) {
  if (sem_wait(&sem_write)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  ++write_count;
  if(write_count == 1) {
    if (sem_wait(&try_read)) {
      perror("sem_wait");
      exit(EXIT_FAILURE);
    }
  }
  if (sem_post(&sem_write)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&resource)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }

  memcpy(&this->key_pressed, &key, sizeof(char));

  if(sem_post(&resource)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
  if (sem_wait(&sem_write)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  --write_count;
  if(write_count == 0) {
    if (sem_post(&try_read)) {
      perror("sem_post");
      exit(EXIT_FAILURE);
    }
  }
  if (sem_post(&sem_write)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
}

time_buffer *createSharedTimerBuffer(const char *path_name) {
  // create shared memory structure
  int fd = shm_open(path_name, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
  if (fd == -1) {
    perror("main: Cannot create shared memory.");
    exit(EXIT_FAILURE);
  }

  // shrink creared shared memory to frameBuffer size
  if (ftruncate(fd, sizeof(time_buffer)) == -1) {
    perror("main: ftruncate error.");
    exit(EXIT_FAILURE);
  }

  // map frame buffer onto shared memory
  time_buffer *shmp = (time_buffer *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                            MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("main: Cannot map memory.");
    exit(EXIT_FAILURE);
  }

  // initialize semaphores as process-shared
  if (sem_init(&shmp->sem_read, 1, 0) == -1) {
    perror("main: Cannot init sem_read.");
    exit(EXIT_FAILURE);
  }

  if (sem_init(&shmp->sem_write, 1, 1) == -1) {
    perror("main: Cannot init sem_write.");
    exit(EXIT_FAILURE);
  }
  return shmp;
}

time_buffer *openSharedTimerBuffer(const char *path_name) {
  // open existing shared memory
  int fd = shm_open(path_name, O_RDWR, 0);
  if (fd == -1) {
    perror("Cannot open shared memory.");
    exit(EXIT_FAILURE);
  }
  time_buffer *shmp = (time_buffer *)mmap(NULL, sizeof(*shmp), PROT_READ | PROT_WRITE,
                                          MAP_SHARED, fd, 0);
  if (shmp == MAP_FAILED) {
    perror("Cannot map memory.");
    exit(EXIT_FAILURE);
  }
  return shmp;
}

void time_buffer::writeBuffer(const int64_t *buffer, size_t buffer_size) {
  // close write semaphore
  if (sem_wait(&sem_write)) {
    perror("sem_wait");
    exit(EXIT_FAILURE);
  }
  memcpy(&this->buffer_, buffer, std::min(buffer_size, BUFFER_SIZE));
  memcpy(&this->size_, &buffer_size, sizeof(buffer_size));
  // open read semaphore
  if (sem_post(&sem_read)) {
    perror("sem_post");
    exit(EXIT_FAILURE);
  }
}

bool time_buffer::tryReadBuffer(int64_t *buffer) {
  // close read semaphore
  if (sem_trywait(&sem_read))
    return false;
  memcpy(buffer, &this->buffer_, std::min(size_, BUFFER_SIZE));
  // open write semaphore
  if (sem_post(&sem_write)) {
    perror("sem_post ");
    exit(EXIT_FAILURE);
  }
  std::cout << "UDA ŁOSIE";
  return true;
}

void time_buffer::readBuffer(int64_t *buffer) {
  // close read semaphore
  if (sem_wait(&sem_read)) {
    perror("sem_wait ");
    exit(EXIT_FAILURE);
  }
  memcpy(buffer, &this->buffer_, std::min(size_, BUFFER_SIZE));
  // open write semaphore
  if (sem_post(&sem_write)) {
    perror("sem_post ");
    exit(EXIT_FAILURE);
  }
  std::cout << "reszta";
}