#include <stdio.h>
#include <assert.h>
#include <nlohmann/json.hpp>
#include "../test-utils.h"
using json = nlohmann::json;

#define DEBUG 0
#define SHARE_TAG 193
#define ROWS1 5
#define COLS1 2
#define ROWS2 4
#define COLS2 2
#define RES_TAG 192

int main(int argc, char** argv) {

  // Checking json file path
  if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <path_to_json_file>" << std::endl;
        return 1;
  }

  // initialize communication
  init(argc, argv); // Establish 3 processes and stores ranking(index of each process) to 'rank' and # of parties to 'num_parties'

  const int rank = get_rank(); // Checks initialization and returns an assigned rank/index for the calling party
  const int pred = get_pred(); // Checks initialization and returns an assigned rank/index of a predecessor party
  const int succ = get_succ(); // Checks initialization and returns an assigned rank/index of successor party

  // Boolean Share (long long) r1: first relation with 2 cols and 5 rows, r2: second relation with 2 cols and 4 rows
  BShare r1s1[ROWS1][COLS1], r1s2[ROWS1][COLS1], r1s3[ROWS1][COLS1],
         r2s1[ROWS2][COLS2], r2s2[ROWS2][COLS2], r2s3[ROWS2][COLS2];

  /** Party1 creates secret shares and secrefy it as s3 and sends them by MPI_Send
     While Party 2 and 3 receives either s1 s2 or s3, so that each party holds one of s1, 2, or 3
     (not like party 1 holds r1s1 and r1s2, and so that operation amongst three in same address can recover the secret)
  **/

  if (rank == 0) { //P1: Party-1
    json js;
    std::ifstream json_file(argv[1]); // Use the first command-line argument as the file path
    if (!json_file.is_open()) {
        std::cerr << "Unable to open file: " << argv[1] << std::endl;
        return 1;
    }
    try {
        json_file >> js;
    } catch (json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }
    json_file.close();

    // Store Original Data (long long) :Initialize input data and shares
    Data r1[ROWS1][COLS1];
    Data r2[ROWS2][COLS2];

    for (int i = 0; i < ROWS1; ++i) {
        r1[i][0] = js["r1"][i][0].get<int>();
        r1[i][1] = js["r1"][i][1].get<int>();
    }
    for (int i = 0; i < ROWS2; ++i) {
        r2[i][0] = js["r2"][i][0].get<int>();
        r2[i][1] = js["r2"][i][1].get<int>();
    }

    init_sharing(); // Runs sodium_init and checks if itinialization of sodium was successful

    // generate r1 shares 
    /**
       1) pass random values to s1 and s2
       2) XOR these randomly generated values
       3) XOR the result of 2) with original secret
       4) The resulting secret is stored in s3
    **/
    for (int i=0; i<ROWS1; i++) { // Iterates over each row of r1
        for (int j=0; j<COLS1; j++) { // Iterates over each column of r1
            // & passes address of the value, instead of the value itself.
            // * inside generate_bool_share reference the address given by & and modifies the value at the address
            generate_bool_share(r1[i][j], &r1s1[i][j], &r1s2[i][j], &r1s3[i][j]);
        }
    }

    // generate r2 shares
    for (int i=0; i<ROWS2; i++) {
        for (int j=0; j<COLS2; j++) {
            generate_bool_share(r2[i][j], &r2s1[i][j], &r2s2[i][j], &r2s3[i][j]);
        }
    }
    /** int MPI_Send(const void *buf, int count, MPI_Datatype datatype, int dest, int tag, MPI_Comm comm);
         - buf: Starting address of the send buffer (the data to be sent).
         - count: Number of elements in the send buffer.
         - datatype: Data type of each send buffer element (e.g., MPI_INT, MPI_FLOAT, MPI_LONG_LONG).
         - dest: Rank of the destination process within the communicator (1 and 2 corresponds to party 2 and 3).
         - tag: Message tag to identify the message (193).
         - comm: Communicator (MPI_COMM_WORLD for all processes is standard).
    **/

    //Send shares to P2 (Second random val generated in generate_bool_share and XORed secret are sent starting from &r1s2[0][0] to all 5*2 vals)
    MPI_Send(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r1s3[0][0], ROWS1*COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s3[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    //Send shares to P3
    MPI_Send(&r1s3[0][0], ROWS1*COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s3[0][0], ROWS2*COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
  }
  else if (rank == 1) { //P2

/**   int MPI_Recv(void *buf, int count, MPI_Datatype datatype, int source, int tag, MPI_Comm comm, MPI_Status *status);
       buf: Starting address of the receive buffer (where the data will be stored).
       count: Number of elements in the receive buffer.
       datatype: Data type of each receive buffer element.
       source: Rank of the source process within the communicator.
       tag: Message tag to identify the message.
       comm: Communicator (usually MPI_COMM_WORLD for all processes).
       status: Status object which contains information about the received message (e.g., source, tag). MPI_STATUS_IGNORE can be used if this information is not needed.
**/

    MPI_Recv(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }
  else { //P3
    MPI_Recv(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  }

  //exchange seeds
  /** sending random bytes to successor(i.e., to party 2 if you are party 1), called seed_local
      and receiving the same from predecessor (i.e., party 3 if you are party 1), called seed_remote
    * Using those seed bytes, set local_state and remote_state with random values
  **/
  exchange_rsz_seeds(succ, pred);

  BShare res[ROWS1*ROWS2]; // join result to be stored in res 5 x 4 size

  // allocate BShare tables 
  // BShareTable: {int ownerId; int partyId; int numRows; int numCols; int relationId; Share **content (2d array) } 
  // It sets 'content' memory space for actual data separately inside allocate_bool ... func
  BShareTable in1 = {-1, rank, ROWS1, 2*COLS1, 1}, in2 = {-1, rank, ROWS2, 2*COLS2, 2};
  allocate_bool_shares_table(&in1); // Passing address of in1 and sets pointers and actual values in contiguous memory address
  allocate_bool_shares_table(&in2);

  // copy shares into the BShareTables
  // store original rand val s1 of own party and same rand val from party 1 in s2
  for (int i=0; i<ROWS1; i++) {
      in1.content[i][0] = r1s1[i][0];
      in1.content[i][1] = r1s2[i][0];
      in1.content[i][2] = r1s1[i][1];
      in1.content[i][3] = r1s2[i][1];
  }
  // relation 2
  for (int i=0; i<ROWS2; i++) {
      in2.content[i][0] = r2s1[i][0];
      in2.content[i][1] = r2s2[i][0];
      in2.content[i][2] = r2s1[i][1];
      in2.content[i][3] = r2s2[i][1];
  }

  /** struct predicate_b {OP_TYPE operation; struct predicate_b *op1; struct predicate_b *op2;
   *    int leftcol; int rightcol; BShare cs1; BShare cs2;   // Used only by EQC, GEQC, and GC
   *  When op1 and/or op2 are NULL, then left and right point to the input shares to be used for this op_type.
  **/
  Predicate_B p = {EQ, NULL, NULL, 0, 0};

  // Equality Check of left and right cols and True or False is stored in res
  join_b(in1, in2, p, res);

  // reveal the result
  Data out[ROWS1*ROWS2]; /** TODO: only rank0 needs to allocate **/
  open_b_array(res, ROWS1*ROWS2, out);

  // assert and print result
  if (rank == 0) {
      std::cout << "[T1, T2]" << std::endl;
      for (int i=0; i<ROWS1*ROWS2; i++) {
          #if DEBUG
            printf("[%d] %lld\t", i, out[i]);
            if (i==0 || i==9 || i==18) {
                assert(out[i] == 1);
            }
            else {
                assert(out[i] == 0);
            }
          #else 
            if(out[i] == 1){
              int t1 = i / ROWS2;
              int t2 = i % ROWS2;
              std::cout << "[" << t1 << ",  " << t2 << "]" << std::endl;
            }
          #endif
      }
      printf("TEST JOIN: OK.\n");
  }
  // tear down communication
  MPI_Finalize();
  return 0;
}
