#include <iostream>
#include <vector>
#include <stdio.h>
#include <assert.h>
#include <nlohmann/json.hpp>
#include "../test-utils.h"
using json = nlohmann::json;

#define DEBUG 0
#define SHARE_TAG 193
#define RESULT_TAG 197

json import_JSON(const std::string& path) {
    json js;
    std::ifstream json_file(path); // Use the first command-line argument as the file path
    if (!json_file.is_open()) {
        std::cerr << "Unable to open file: " << path << std::endl;
        return 1;
    }
    try {
        json_file >> js;
    } catch (json::parse_error& e) {
        std::cerr << "Parse error: " << e.what() << std::endl;
        return 1;
    }
    json_file.close();
    return js;
}


int main(int argc, char** argv) {

  // Checking json file path
  if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_json_file for P1>" << " <path_to_json_file for P2>" << std::endl;
        return 1;
  }

  // initialize communication
  init(argc, argv); // Establish 3 processes and stores ranking(index of each process) to 'rank' and # of parties to 'num_parties'

  const int rank = get_rank(); // Checks initialization and returns an assigned rank/index for the calling party
  const int pred = get_pred(); // Checks initialization and returns an assigned rank/index of a predecessor party
  const int succ = get_succ(); // Checks initialization and returns an assigned rank/index of successor party

  json js1 = import_JSON(argv[1]);
  json js2 = import_JSON(argv[2]);
  int ROWS1 = static_cast<int>(js1["r1"].size());
  int COLS1 = static_cast<int>(js1["r1"][0].size());
  int ROWS2 = static_cast<int>(js2["r2"].size());
  int COLS2 = static_cast<int>(js2["r2"][0].size());

  // Boolean Share (long long) r1: first relation with 2 cols and 5 rows, r2: second relation with 2 cols and 4 rows
  BShare r1s1[ROWS1][COLS1], r1s2[ROWS1][COLS1], r1s3[ROWS1][COLS1],
         r2s1[ROWS2][COLS2], r2s2[ROWS2][COLS2], r2s3[ROWS2][COLS2];

  /** Party1 creates secret shares and secrefy it as s3 and sends them by MPI_Send
     While Party 2 and 3 receives either s1 s2 or s3, so that each party holds one of s1, 2, or 3
     (not like party 1 holds r1s1 and r1s2, and so that operation amongst three in same address can recover the secret)
  **/

  if (rank == 0) { //P1: Party-1
    init_sharing(); // Runs sodium_init and checks if itinialization of sodium was successful

    Data r1[ROWS1][COLS1];

    for (int i = 0; i < ROWS1; ++i) {
        r1[i][0] = js1["r1"][i][0].get<int>();
        r1[i][1] = js1["r1"][i][1].get<int>();
    }

    // generate r1 shares 
    for (int i=0; i<ROWS1; i++) { // Iterates over each row of r1
        for (int j=0; j<COLS1; j++) { // Iterates over each column of r1
            generate_bool_share(r1[i][j], &r1s1[i][j], &r1s2[i][j], &r1s3[i][j]);
        }
    }

    //Send shares to P2 (Second random val generated in generate_bool_share and XORed secret are sent starting from &r1s2[0][0] to all 5*2 vals)
    MPI_Send(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r1s3[0][0], ROWS1*COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
    
    //Send shares to P3
    MPI_Send(&r1s3[0][0], ROWS1*COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);

    //Receive from P2
    MPI_Recv(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
  }
  else if (rank == 1) { //P2

    Data r2[ROWS2][COLS2];

    for (int i = 0; i < ROWS2; ++i) {
        r2[i][0] = js2["r2"][i][0].get<int>();
        r2[i][1] = js2["r2"][i][1].get<int>();
    }

    // generate r2 shares
    for (int i=0; i<ROWS2; i++) {
        for (int j=0; j<COLS2; j++) {
            generate_bool_share(r2[i][j], &r2s1[i][j], &r2s2[i][j], &r2s3[i][j]);
        }
    }

    // Receive from P1
    MPI_Recv(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
    //Send shares to P1
    MPI_Send(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD);
    
    //Send shares to P3
    MPI_Send(&r2s3[0][0], ROWS2*COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
    MPI_Send(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);


    // Copy data using nested for loops
    for (int i = 0; i < ROWS2; ++i) {
        for (int j = 0; j < COLS2; ++j) {
            r2s1[i][j] = r2s2[i][j];
        }
    }

    for (int i = 0; i < ROWS2; ++i) {
        for (int j = 0; j < COLS2; ++j) {
            r2s2[i][j] = r2s3[i][j];
        }
    }
  }
  else { //P3
    // Receive from P1
    MPI_Recv(&r1s1[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r1s2[0][0], ROWS1*COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Receive from P2
    MPI_Recv(&r2s1[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    MPI_Recv(&r2s2[0][0], ROWS2*COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    
  }

  MPI_Barrier(MPI_COMM_WORLD);

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
      #if DEBUG
        for (int i=0; i<ROWS1*ROWS2; i++) {
          printf("[%d] %lld\t", i, out[i]);
          if (i==0 || i==9 || i==18) {
              assert(out[i] == 1);
          }
          else {
              assert(out[i] == 0);
          }
        }
      #else 
        // Store indices
        std::vector<int> t1_index;
        std::vector<int> t2_index;
        for (int i=0; i<ROWS1*ROWS2; i++) {
          if(out[i] == 1){
            int t1 = i / ROWS2;
            int t2 = i % ROWS2;
            t1_index.push_back(t1);
            t2_index.push_back(t2);
          }
        }

        // Send Size of t2 index
        int size_to_send = t2_index.size();
        MPI_Send(&size_to_send, 1, MPI_INT, 1, RESULT_TAG, MPI_COMM_WORLD);
        MPI_Send(t2_index.data(), size_to_send, MPI_INT, 1, RESULT_TAG, MPI_COMM_WORLD);

        std::cout << "/// Joined Table ///" << std::endl;
        for (int i=0; i<size_to_send; i++) {
            int t1 = t1_index[i];
            long long val0 = js1["r1"][t1][0];
            long long val1 = js1["r1"][t1][1];
            
            // long long rec_val;
            long long rec_val;
            MPI_Recv(&rec_val, 1, MPI_LONG_LONG, 1, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "[" << val0 << ", " << val1 << ", " << rec_val<<  "]" << std::endl;
        }
      #endif
  } 
  else if (rank == 1){
        
        // Receive size of t2 index
        int size_to_receive;
        MPI_Recv(&size_to_receive, 1, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        
        // Receive the actual t2_index
        std::vector<int> t2_index(size_to_receive);
        MPI_Recv(t2_index.data(), size_to_receive, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        for (int i=0; i<size_to_receive; i++) {
          int t2 = t2_index[i];

          // send back to p1
          long long send_val = js2["r2"][t2][1];
          MPI_Send(&send_val, 1, MPI_LONG_LONG, 0, RESULT_TAG, MPI_COMM_WORLD);
        }
  }

  // tear down communication
  MPI_Finalize();
  return 0;
}
