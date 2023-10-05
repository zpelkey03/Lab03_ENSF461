#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <limits.h>



// TODO: Add more fields to this struct
struct job {
    int id;
    int arrival;
    int length;
    struct job *next;
    int responseTime;
    int turnaroundTime;
    int waitTime;
};

/*** Globals ***/ 
int seed = 100;


//This is the start of our linked list of jobs, i.e., the job list
struct job *head = NULL;

/*** Globals End ***/

/*Function to append a new job to the list*/
void append(int id, int arrival, int length){
  // create a new struct and initialize it with the input data
  struct job *tmp = (struct job*) malloc(sizeof(struct job));

  //tmp->id = numofjobs++;
  tmp->id = id;
  tmp->length = length;
  tmp->arrival = arrival;

  // the new job is the last job
  tmp->next = NULL;

  // Case: job is first to be added, linked list is empty 
  if (head == NULL){
    head = tmp;
    return;
  }

  struct job *prev = head;

  //Find end of list 
  while (prev->next != NULL){
    prev = prev->next;
  }

  //Add job to end of list 
  prev->next = tmp;
  return;
}


/*Function to read in the workload file and create job list*/
void read_workload_file(char* filename) {
  int id = 0;
  FILE *fp;
  size_t len = 0;
  ssize_t read;
  char *line = NULL,
       *arrival = NULL, 
       *length = NULL;

  struct job **head_ptr = malloc(sizeof(struct job*));

  if( (fp = fopen(filename, "r")) == NULL)
    exit(EXIT_FAILURE);

  while ((read = getline(&line, &len, fp)) > 1) {
    arrival = strtok(line, ",\n");
    length = strtok(NULL, ",\n");
       
    // Make sure neither arrival nor length are null. 
    assert(arrival != NULL && length != NULL);
        
    append(id++, atoi(arrival), atoi(length));
  }

  fclose(fp);

  // Make sure we read in at least one job
  assert(id > 0);

  return;
}


void policy_FIFO(struct job *head) {
    struct job *current = head;
    int time = 0;

    printf("Execution trace with FIFO:\n");

    while (current != NULL) {
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", time, current->id, current->arrival, current->length);
        time += current->length;
        current = current->next;
    }

    printf("End of execution with FIFO.\n");
    return;
}

// Function to sort the jobs by ID
void sortJobsById(struct job **head) {
    struct job *current = *head;
    struct job *nextJob = NULL;
    int swapped;

    if (*head == NULL) {
        return; // No jobs to sort
    }

    do {
        swapped = 0;
        current = *head;

        while (current->next != nextJob) {
            if (current->id > current->next->id) {
                // Swap current job and next job
                int tempId = current->id;
                int tempArrival = current->arrival;
                int tempLength = current->length;
                int tempResponseTime = current->responseTime;
                int tempTurnaroundTime = current->turnaroundTime;
                int tempWaitTime = current->waitTime;

                current->id = current->next->id;
                current->arrival = current->next->arrival;
                current->length = current->next->length;
                current->responseTime = current->next->responseTime;
                current->turnaroundTime = current->next->turnaroundTime;
                current->waitTime = current->next->waitTime;

                current->next->id = tempId;
                current->next->arrival = tempArrival;
                current->next->length = tempLength;
                current->next->responseTime = tempResponseTime;
                current->next->turnaroundTime = tempTurnaroundTime;
                current->next->waitTime = tempWaitTime;

                swapped = 1;
            }
            current = current->next;
        }
        nextJob = current;
    } while (swapped);
}

void analyze_FIFO(struct job *head) {
    if (head == NULL) {
        printf("No jobs to analyze.\n");
        return;
    }

    struct job *current = head;
    int currentTime = 0;
    int totalResponseTime = 0;
    int totalTurnaroundTime = 0;
    int totalWaitTime = 0;


    while (current != NULL) {

        // Calculate Response Time
        int responseTime = currentTime - current->arrival;
        current->responseTime = responseTime;

        // Calculate Turnaround Time
        int turnaroundTime = responseTime + current->length;
        current-> turnaroundTime = turnaroundTime;

        // Calculate Wait Time
        int waitTime = responseTime;
        current-> waitTime = waitTime;

        totalResponseTime += responseTime;
        totalTurnaroundTime += turnaroundTime;
        totalWaitTime += waitTime;

        currentTime += current->length;
        current = current->next;
    }

    sortJobsById(&head);
    
    current = head;
    while (current != NULL) {
        printf("Job %d -- ", current->id);
        printf("Response time: %d  ", current->responseTime);
        printf("Turnaround: %d  ", current->turnaroundTime);
        printf("Wait: %d\n", current->waitTime);

        current = current->next;
      }

    int numJobs = 0; // Count the number of jobs
    current = head;
    while (current != NULL) {
        numJobs++;
        current = current->next;
    }

    // Calculate and print averages
    double avgResponseTime = (double)totalResponseTime / numJobs;
    double avgTurnaroundTime = (double)totalTurnaroundTime / numJobs;
    double avgWaitTime = (double)totalWaitTime / numJobs;

    printf("Average -- Response: %.2lf  Turnaround %.2lf  Wait %.2lf\n", avgResponseTime, avgTurnaroundTime, avgWaitTime);

}

// Function to sort the jobs by length (SJF)
void sortJobsByLength(struct job **head) {
    struct job *current = *head;
    struct job *nextJob = NULL;
    int swapped;

    if (*head == NULL) {
        return; // No jobs to sort
    }

    do {
        swapped = 0;
        current = *head;

        while (current->next != nextJob) {
            if (current->length > current->next->length) {
                // Swap current job and next job
                int tempId = current->id;
                int tempArrival = current->arrival;
                int tempLength = current->length;

                current->id = current->next->id;
                current->arrival = current->next->arrival;
                current->length = current->next->length;

                current->next->id = tempId;
                current->next->arrival = tempArrival;
                current->next->length = tempLength;

                swapped = 1;
            }
            current = current->next;
        }
        nextJob = current;
    } while (swapped);
}


// Function to sort the jobs by length and arrival time (SJF)
void sortJobsBySJF(struct job **head) {
    struct job *current = *head;
    struct job *nextJob = NULL;
    int swapped;

    if (*head == NULL) {
        return; // No jobs to sort
    }

    do {
        swapped = 0;
        current = *head;

        while (current->next != nextJob) {
            // Sort by arrival time first
            if (current->arrival > current->next->arrival) {
                // Swap current job and next job
                int tempId = current->id;
                int tempArrival = current->arrival;
                int tempLength = current->length;

                current->id = current->next->id;
                current->arrival = current->next->arrival;
                current->length = current->next->length;

                current->next->id = tempId;
                current->next->arrival = tempArrival;
                current->next->length = tempLength;

                swapped = 1;
            }
            // If arrival times are equal, sort by length
            else if (current->arrival == current->next->arrival && current->length > current->next->length) {
                // Swap current job and next job
                int tempId = current->id;
                int tempArrival = current->arrival;
                int tempLength = current->length;

                current->id = current->next->id;
                current->arrival = current->next->arrival;
                current->length = current->next->length;

                current->next->id = tempId;
                current->next->arrival = tempArrival;
                current->next->length = tempLength;

                swapped = 1;
            }
            current = current->next;
        }
        nextJob = current;
    } while (swapped);
}


void policy_SJF(struct job *head) {
    if (head == NULL) {
        printf("No jobs to schedule.\n");
        return;
    }

    sortJobsBySJF(&head);

    struct job *current = head;
    int currentTime = 0;

    printf("Execution trace with SJF:\n");

    while (current != NULL) {
        printf("t=%d: [Job %d] arrived at [%d], ran for: [%d]\n", currentTime, current->id, current->arrival, current->length);
        currentTime += current->length;
        current = current->next;
    }

    printf("End of execution with SJF.\n");
}

void analyze_SJF(struct job *head) {
    if (head == NULL) {
        printf("No jobs to analyze.\n");
        return;
    }

    sortJobsBySJF(&head);

    struct job *current = head;
    int currentTime = 0;
    int totalResponseTime = 0;
    int totalTurnaroundTime = 0;
    int totalWaitTime = 0;


    while (current != NULL) {
        printf("Job %d -- ", current->id);

        // Calculate Response Time
        int responseTime = currentTime - current->arrival;
        printf("Response time: %d  ", responseTime);

        // Calculate Turnaround Time
        int turnaroundTime = responseTime + current->length;
        printf("Turnaround: %d  ", turnaroundTime);

        // Calculate Wait Time
        int waitTime = responseTime;
        printf("Wait: %d\n", waitTime);

        totalResponseTime += responseTime;
        totalTurnaroundTime += turnaroundTime;
        totalWaitTime += waitTime;

        currentTime += current->length;
        current = current->next;
    }

    int numJobs = 0; // Count the number of jobs
    current = head;
    while (current != NULL) {
        numJobs++;
        current = current->next;
    }

    // Calculate and print averages
    double avgResponseTime = (double)totalResponseTime / numJobs;
    double avgTurnaroundTime = (double)totalTurnaroundTime / numJobs;
    double avgWaitTime = (double)totalWaitTime / numJobs;

    printf("Average -- Response: %.2lf  Turnaround %.2lf  Wait %.2lf\n", avgResponseTime, avgTurnaroundTime, avgWaitTime);

}

int main(int argc, char **argv) {

 if (argc < 4) {
    fprintf(stderr, "missing variables\n");
    fprintf(stderr, "usage: %s analysis-flag policy workload-file\n", argv[0]);
		exit(EXIT_FAILURE);
  }

  int analysis = atoi(argv[1]);
  char *policy = argv[2],
       *workload = argv[3];

  // Note: we use a global variable to point to 
  // the start of a linked-list of jobs, i.e., the job list 
  read_workload_file(workload);

  if (strcmp(policy, "FIFO") == 0 ) {
    policy_FIFO(head);
    if (analysis) {
      printf("Begin analyzing FIFO:\n");
      analyze_FIFO(head);
      printf("End analyzing FIFO.\n");
    }

    exit(EXIT_SUCCESS);
  }

  // TODO: Add other policies 
    if (strcmp(policy, "SJF") == 0 ) {
    policy_SJF(head);
    if (analysis) {
      printf("Begin analyzing SJF:\n");
      analyze_SJF(head);
      printf("End analyzing SJF.\n");
    }

    exit(EXIT_SUCCESS);
  }
	exit(EXIT_SUCCESS);
}
