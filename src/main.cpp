#include "SharedMemory.hpp"
#include "SnakeGame.hpp"

int main()
{
    SnakeGame snake_game;
    
    do
    {
        do{} while(snake_game.drawStartWindow());
        while (!snake_game.isEndGame()){
            snake_game.processImage();
            snake_game.drawGameWindow();
        }
        do{}while(snake_game.drawEndWindow());

    } while (snake_game.isRepeat());
    return 0;

}

/* const char *SHARED_MEMORY_PATH = "/frame_buffer";

void initProcess(void (*fun)())
{
    if (fork() == 0)
    {
        fun();
        exit(EXIT_SUCCESS);
    }
}

// odczyt kamery: klatka -> pamiec wspoldzielona
void CameraRead()
{
    // cv::Mat frame;
    // std::function<void(const char*,const size_t)> sendData, receiveData;

    // switch(mode):
    //  case 0: 
    //     sendData = &SHM_write; //tmp names
    //     receiveData = &SHM_read;
    //     break;
    //  case 1:
    //     sendData = &MSGQ_write; //tmp names
    //     receiveData = &MSGQ_read;
    //     break;
    //  case 1:
    //     sendData = &PIPE_write; //tmp names
    //     receiveData = &PIPE_read;
    //     break;

    //sendData(frame);
    //sendData(&frame,frame.size().area());

    sh_m *shmp = openSharedMemory(SHARED_MEMORY_PATH);

    for (int i = 0; i < 10; ++i)
    {
        // close write semaphore
        if (sem_wait(&shmp->sem_write))
        {
            perror("A: sem_wait error.");
            exit(EXIT_FAILURE);
        }

        // writing data into shared memory
        std::string str = "A frame #" + std::to_string(i);
        std::cout << "A: Writing data: " << str << std::endl;
        const char *chr = str.c_str();
        memcpy(&shmp->frame, chr, strlen(chr));

        // open read semaphore
        if (sem_post(&shmp->sem_read))
        {
            perror("A: sem_post error.");
            exit(EXIT_FAILURE);
        }

        // data processing
        sleep(1); // simulates data processing
    }

    // clear after process is finished
    shm_unlink(SHARED_MEMORY_PATH);
}

void GameCalculate()
{
    sh_m *shmp = openSharedMemory(SHARED_MEMORY_PATH);

    for (int i = 0; i < 10; ++i)
    {
        // close read semaphore
        if (sem_wait(&shmp->sem_read))
        {
            perror("B: sem_wait error.");
            exit(EXIT_FAILURE);
        }

        // reading data from shared memory
        printf("B: Reading data: %s\n", &shmp->frame);

        // open write semaphore
        if (sem_post(&shmp->sem_write))
        {
            perror("B: sem_post error.");
            exit(EXIT_FAILURE);
        }

        // data processing
        sleep(1); // simulates data processing
    }

    // clear after process is finished
    shm_unlink(SHARED_MEMORY_PATH);
}
void GameDraw()
{
    cv::Mat game_frame;
    sh_m *shmp = openSharedMemory(SHARED_MEMORY_PATH);

    for (int i = 0; i < 10; ++i)
    {
        // close read semaphore
        if (sem_wait(&shmp->sem_read))
        {
            perror("B: sem_wait error.");
            exit(EXIT_FAILURE);
        }

        // reading data from shared memory
        printf("B: Reading data: %s\n", &shmp->frame);

        // open write semaphore
        if (sem_post(&shmp->sem_write))
        {
            perror("B: sem_post error.");
            exit(EXIT_FAILURE);
        }

        // data processing
        cv::imshow(window_game_name,game_frame);
    }

    // clear after process is finished
    shm_unlink(SHARED_MEMORY_PATH);
}


int main()
{
    // unlink from shared memory object if still exists
    shm_unlink(SHARED_MEMORY_PATH);

    createSharedMemory(SHARED_MEMORY_PATH);

    //initProcess(processA);
    //initProcess(processB);
    //sleep(10);
    // unlink from shared memory object
    shm_unlink(SHARED_MEMORY_PATH);
    exit(EXIT_SUCCESS);
}
 */