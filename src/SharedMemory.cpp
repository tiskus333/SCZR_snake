#include "SharedMemory.hpp"

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

void SharedMemory::sendToSharedMemory(const unsigned char *p_data,
                                      const size_t data_size,
                                      const int offset) {
  // close write semaphore
  if (sem_wait(&sem_write)) {
    perror("A: sem_wait error.");
    exit(EXIT_FAILURE);
  }
  memcpy(&this->frame[offset], p_data, std::min(this->size, data_size));
  // open read semaphore
  if (sem_post(&sem_read)) {
    perror("A: sem_post error.");
    exit(EXIT_FAILURE);
  }
}

void SharedMemory::receiveFromSharedMemory(unsigned char *p_write_to,
                                           const size_t data_size,
                                           const int offset) {
  // close read semaphore
  if (sem_wait(&sem_read)) {
    perror("A: sem_wait error.");
    exit(EXIT_FAILURE);
  }
  memcpy(p_write_to, &this->frame[offset], std::min(this->size, data_size));
  // open write semaphore
  if (sem_post(&sem_write)) {
    perror("A: sem_post error.");
    exit(EXIT_FAILURE);
  }
}
void SharedMemory::readFrame(const cv::Mat &frame) {
  // receiving data from shared memory
  receiveFromSharedMemory(frame.data, DATA_SIZE);
}

void SharedMemory::writeFrame(const cv::Mat &frame) {
  // writing data into shared memory
  sendToSharedMemory(frame.data, DATA_SIZE);
}
void SharedMemory::readKey(unsigned char &key) {
  // receiving data from shared memory
  receiveFromSharedMemory(&key, 1, DATA_SIZE);
}
void SharedMemory::writeKey(const unsigned char &key) {
  // writing data into shared memory
  sendToSharedMemory(&key, 1, DATA_SIZE);
}