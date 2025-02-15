/****************************************/
/*                                      */
/*                                      */
/*  Game of Life with Pthread and CUDA  */
/*                                      */
/*  ECE521 Project #2                   */
/*  @ Korea University                  */
/*                                      */
/*                                      */
/****************************************/

#include "glife.h"
using namespace std;

int gameOfLife(int argc, char *argv[]);
void singleThread(int, int, int);
void* workerThread(void *);
int nprocs;
GameOfLifeGrid* g_GameOfLifeGrid;
pthread_barrier_t barrier;

uint64_t dtime_usec(uint64_t start)
{
  timeval tv;
  gettimeofday(&tv, 0);
  return ((tv.tv_sec*USECPSEC)+tv.tv_usec)-start;
}

GameOfLifeGrid::GameOfLifeGrid(int rows, int cols, int gen)
{
  m_Generations = gen;
  m_Rows = rows;
  m_Cols = cols;

  m_Grid = (int**)malloc(sizeof(int*) * rows);
  if (m_Grid == NULL) 
    cout << "1 Memory allocation error " << endl;

  m_Temp = (int**)malloc(sizeof(int*) * rows);
  if (m_Temp == NULL) 
    cout << "2 Memory allocation error " << endl;

  m_Grid[0] = (int*)malloc(sizeof(int) * (cols*rows));
  if (m_Grid[0] == NULL) 
    cout << "3 Memory allocation error " << endl;

  m_Temp[0] = (int*)malloc(sizeof(int) * (cols*rows));	
  if (m_Temp[0] == NULL) 
    cout << "4 Memory allocation error " << endl;

  for (int i = 1; i < rows; i++) {
    m_Grid[i] = m_Grid[i-1] + cols;
    m_Temp[i] = m_Temp[i-1] + cols;
  }

  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < cols; j++) {
      m_Grid[i][j] = m_Temp[i][j] = 0;
    }
  }
}

// Entry point
int main(int argc, char* argv[])
{
  if (argc != 7) {
    cout <<"Usage: " << argv[0] << " <input file> <display> <nprocs>"
           " <# of generation> <width> <height>" << endl;
    cout <<"\n\tnprocs = 0: Enable GPU" << endl;
    cout <<"\tnprocs > 0: Run on CPU" << endl;
    cout <<"\tdisplay = 1: Dump results" << endl;
    return 1;
  }

  return gameOfLife(argc, argv);
}

int gameOfLife(int argc, char* argv[])
{
  int cols, rows, gen;
  ifstream inputFile;
  int input_row, input_col, display;
  uint64_t difft, cuda_difft;
  pthread_t *threadID;

  inputFile.open(argv[1], ifstream::in);

  if (inputFile.is_open() == false) {
    cout << "The "<< argv[1] << " file can not be opend" << endl;
    return 1;
  }

  display = atoi(argv[2]);
  nprocs = atoi(argv[3]);
  gen = atoi(argv[4]);
  cols = atoi(argv[5]);
  rows = atoi(argv[6]);

  g_GameOfLifeGrid = new GameOfLifeGrid(rows, cols, gen);

  while (inputFile.good()) {
    inputFile >> input_row >> input_col;
    if (input_row >= rows || input_col >= cols) {
      cout << "Invalid grid number" << endl;
      return 1;
    } else
      g_GameOfLifeGrid->setCell(input_row, input_col);
  }

  // Start measuring execution time
  difft = dtime_usec(0);

  // TODO: YOU NEED TO IMPLMENT THE SINGLE THREAD, PTHREAD, AND CUDA
  if (nprocs == 0) {
    // Running on GPU
    cuda_difft = runCUDA(rows, cols, gen, g_GameOfLifeGrid, display);
  } else if (nprocs == 1) {
    // Running a single thread
    singleThread(rows, cols, gen);
  } else { 
    // Running multiple threads (pthread)
    threadID = new pthread_t[nprocs];
    int* threadArgs = new int[nprocs];
    
    pthread_barrier_init(&barrier, NULL, nprocs);
    
    for (int i = 0; i < nprocs; i++) {
      threadArgs[i] = i;
      pthread_create(&threadID[i], NULL, workerThread, &threadArgs[i]);
    }
    
    for (int i = 0; i < nprocs; i++) {
      pthread_join(threadID[i], NULL);
    }
    
    pthread_barrier_destroy(&barrier);
    delete[] threadID;
    delete[] threadArgs;
  }

  difft = dtime_usec(difft);

  // Print indices only for running on CPU(host).
  if (display && nprocs) {
    g_GameOfLifeGrid->dump();
    g_GameOfLifeGrid->dumpIndex();
  }

  if (nprocs) {
    // Single or multi-thread execution time 
    cout << "Execution time(seconds): " << difft/(float)USECPSEC << endl;
  } else {
    // CUDA execution time
    cout << "Execution time(seconds): " << cuda_difft/(float)USECPSEC << endl;
  }
  inputFile.close();
  cout << "Program end... " << endl;
  return 0;
}

// TODO: YOU NEED TO IMPLMENT SINGLE THREAD
void singleThread(int rows, int cols, int gen)
{
  for (int i = 0; i < gen; i++) {
    g_GameOfLifeGrid->next(); // do one gen
  }

  g_GameOfLifeGrid->dump(); // print final state for debuging
}

// TODO: YOU NEED TO IMPLMENT PTHREAD
void* workerThread(void *arg)
{
  int thread_id = *(int*)arg;
  int rows_per_thread = g_GameOfLifeGrid->getRows() / nprocs;
  int start_row = thread_id * rows_per_thread;
  int end_row = (thread_id == nprocs -1) ? g_GameOfLifeGrid->getRows() : start_row + rows_per_thread;
  
  for (int gen = 0; gen < g_GameOfLifeGrid->getGens(); gen++) {
    g_GameOfLifeGrid->next(start_row, end_row);
    pthread_barrier_wait(&barrier); // waiting until all thread completed
    
    if (thread_id == 0) 
      g_GameOfLifeGrid->swapGrids(); // update grid
      
    pthread_barrier_wait(&barrier); // synchronize again
  }
  
  pthread_exit(NULL);
}

void GameOfLifeGrid::swapGrids()
{
    std::swap(m_Grid, m_Temp);
}

// HINT: YOU MAY NEED TO FILL OUT BELOW FUNCTIONS OR CREATE NEW FUNCTIONS
void GameOfLifeGrid::next(const int from, const int to)
{
  for (int i = from; i < to; i++) {
    for (int j = 0; j < m_Cols; j++) {
      int neighbors = getNumOfNeighbors(i, j);
      
      // Conway's Game of Life rules
      if (m_Grid[i][j] == LIVE) 
        m_Temp[i][j] = (neighbors == 2 || neighbors == 3) ? LIVE : DEAD;
      else 
        m_Temp[i][j] = (neighbors == 3) ? LIVE : DEAD;
    }
  }
}

void GameOfLifeGrid::next()
{
  for (int i = 0; i < m_Rows; i++) {
    for (int j = 0; j < m_Cols; j++) {
      int neighbors = getNumOfNeighbors(i, j);
			
      // Conway's Game of Life rules
      if (m_Grid[i][j] == LIVE) {
        if (neighbors < 2 || neighbors > 3) 
          dead(i, j);
        else 
          live(i, j);
      } else {
        if  (neighbors == 3)
          live(i, j);
        else 
          dead(i, j);
      }
    }
  }
	
  // set next gen
  swap(m_Grid, m_Temp);
}

// TODO: YOU MAY NEED TO IMPLMENT IT TO GET NUMBER OF NEIGHBORS 
int GameOfLifeGrid::getNumOfNeighbors(int rows, int cols)
{
  int numOfNeighbors = 0;
  int dx[] = {-1, -1, -1, 0, 0, 1, 1, 1};
  int dy[] = {-1, 0, 1, -1, 1, -1, 0, 1};
  
  for (int i = 0; i < 8; i++) {
    int newRow = rows + dx[i];
    int newCol = cols + dy[i];
    
    if (newRow >= 0 && newRow < m_Rows && newCol >= 0 && newCol < m_Cols) {
      numOfNeighbors += m_Grid[newRow][newCol];
    }
  }

  return numOfNeighbors;
}

void GameOfLifeGrid::dump() 
{
  cout << "===============================" << endl;

  for (int i = 0; i < m_Rows; i++) {
    cout << "[" << i << "] ";
    for (int j = 0; j < m_Cols; j++) {
      if (m_Grid[i][j] == 1)
        cout << "*";
      else
        cout << "o";
    }
    cout << endl;
  }
  cout << "===============================\n" << endl;
}

void GameOfLifeGrid::dumpIndex()
{
  cout << ":: Dump Row Column indices" << endl;
  for (int i=0; i < m_Rows; i++) {
    for (int j=0; j < m_Cols; j++) {
      if (m_Grid[i][j]) cout << i << " " << j << endl;
    }
  }
}
