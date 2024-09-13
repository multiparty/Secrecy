#include <assert.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <vector>
#include "../test-utils.h"
#include <jsoncons/json.hpp>
#include <jsoncons_ext/csv/csv.hpp>
using namespace jsoncons;
#define INIT_TAG 191
#define SHARE_TAG 193
#define RESULT_TAG 197

void download_from_s3(int rank, const std::string& filename) {
    std::string rankStr = std::to_string(rank);
    std::string awsCommand = "aws s3 cp s3://secrecy-bucket" + rankStr + "/" + filename + " ../";
    int result = system(awsCommand.c_str());

    // Check if the command executed successfully
    if (result == 0) {
        std::cout << "File downloaded successfully from " << " secrecy-bucket" << rankStr << std::endl;
    } else {
        std::cerr << "Error downloading file. Command returned: " << result << std::endl;
        exit(EXIT_FAILURE); 
    }
}

void upload_to_s3(int rank, json output_json, const std::string& filename){
        // Convert the rank and filename to Strings
        std::string rankStr = std::to_string(rank+1);
        std::ofstream json_file(filename);
        if(!json_file.is_open()){
            std::cerr << "Error openiing file: " << filename << std::endl;
            return;
        }
        
        // Copy the content to output json file
        json_file << pretty_print(output_json);
        json_file.close();

        // Run the command
        std::string awsUploadCommand = "aws s3 cp " + filename + " s3://secrecy-bucket" + rankStr +"/";
        int result = system(awsUploadCommand.c_str());
        if (result == 0) {
            std::cout << "File uploaded successfully to " << "secrecy-bucket" << rankStr <<std::endl;
        } else {
            std::cerr << "Error uploading file. Command returned: " << result << std::endl;
        }
}

int main(int argc, char** argv) {
    // Checking json file path
    if (argc < 3) {
        std::cerr << "Usage: " << argv[0] << " <path_to_csv_file for P1>"
                  << " <path_to_json_file for P2>" << std::endl;
        return 1;
    }

    // Setting Up variables
    int ROWS1;
    int COLS1;
    int ROWS2;
    int COLS2;

    // initialize communication
    init(argc, argv);  // Establish 3 processes and stores ranking(index of each process) to 'rank'
                       // and # of parties to 'num_parties'

    const int rank = get_rank();  // Checks initialization and returns an assigned rank/index for
                                  // the calling party
    const int pred = get_pred();  // Checks initialization and returns an assigned rank/index of a
                                  // predecessor party
    const int succ = get_succ();  // Checks initialization and returns an assigned rank/index of successor party
    
    // Common csv option for party1 and 2
    auto options = csv::csv_options{}.assume_header(false).mapping_kind(csv::csv_mapping_kind::n_rows);
    if (rank == 0) {     // P1: Party-1
        std::string filename = argv[1];
        if (filename.substr(filename.find_last_of(".") + 1) != "csv") {
            std::cerr << "Error: The file '" << filename << "' is not a CSV file. Please provide a .csv file." << std::endl;
            return 1;
        }
        download_from_s3(rank + 1, filename);
        init_sharing();  // Runs sodium_init and checks if itinialization of sodium was successful
        std::string csv_file = "./../" + filename;
        std::ifstream is1(csv_file);
        ojson js1 = csv::decode_csv<ojson>(is1,options);
        ROWS1 = static_cast<int>(js1.size());
        COLS1 = static_cast<int>(js1[0].size());
        //////// Send ROWS1 and COLS1 to P2 and 3
        MPI_Send(&ROWS1, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&COLS1, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&ROWS1, 1, MPI_INT, 2, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&COLS1, 1, MPI_INT, 2, INIT_TAG, MPI_COMM_WORLD);

        //////// Receive ROWS2 and COLS2 from P2
        MPI_Recv(&ROWS2, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&COLS2, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);

        BShare r1s1[ROWS1][COLS1], r1s2[ROWS1][COLS1], r1s3[ROWS1][COLS1], r2s1[ROWS2][COLS2], r2s2[ROWS2][COLS2], r2s3[ROWS2][COLS2];
        Data r1[ROWS1][COLS1];

        for (int i = 0; i < ROWS1; ++i) {
            r1[i][0] = js1[i][0].as<int>();
            r1[i][1] = js1[i][1].as<int>();
        }

        // generate r1 shares
        for (int i = 0; i < ROWS1; i++) {      // Iterates over each row of r1
            for (int j = 0; j < COLS1; j++) {  // Iterates over each column of r1
                generate_bool_share(r1[i][j], &r1s1[i][j], &r1s2[i][j], &r1s3[i][j]);
            }
        }

        // Send shares to P2 (Second random val generated in generate_bool_share and XORed secret
        // are sent starting from &r1s2[0][0] to all 5*2 vals)
        MPI_Send(&r1s2[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);
        MPI_Send(&r1s3[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD);

        // Send shares to P3
        MPI_Send(&r1s3[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
        MPI_Send(&r1s1[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);

        // Receive from P2
        MPI_Recv(&r2s1[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r2s2[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);

        // exchange seeds
        /** sending random bytes to successor(i.e., to party 2 if you are party 1), called
        seed_local and receiving the same from predecessor (i.e., party 3 if you are party 1),
        called seed_remote
          * Using those seed bytes, set local_state and remote_state with random values
        **/
        exchange_rsz_seeds(succ, pred);

        BShare res[ROWS1 * ROWS2];  // join result to be stored in res 5 x 4 size

        // allocate BShare tables
        // BShareTable: {int ownerId; int partyId; int numRows; int numCols; int relationId; Share
        // **content (2d array) } It sets 'content' memory space for actual data separately inside
        // allocate_bool ... func
        BShareTable in1 = {-1, rank, ROWS1, 2 * COLS1, 1}, in2 = {-1, rank, ROWS2, 2 * COLS2, 2};
        allocate_bool_shares_table(&in1);  // Passing address of in1 and sets pointers and actual
                                           // values in contiguous memory address
        allocate_bool_shares_table(&in2);

        // copy shares into the BShareTables
        // store original rand val s1 of own party and same rand val from party 1 in s2
        for (int i = 0; i < ROWS1; i++) {
            in1.content[i][0] = r1s1[i][0];
            in1.content[i][1] = r1s2[i][0];
            in1.content[i][2] = r1s1[i][1];
            in1.content[i][3] = r1s2[i][1];
        }
        // relation 2
        for (int i = 0; i < ROWS2; i++) {
            in2.content[i][0] = r2s1[i][0];
            in2.content[i][1] = r2s2[i][0];
            in2.content[i][2] = r2s1[i][1];
            in2.content[i][3] = r2s2[i][1];
        }

        /** struct predicate_b {OP_TYPE operation; struct predicate_b *op1; struct predicate_b *op2;
         *    int leftcol; int rightcol; BShare cs1; BShare cs2;   // Used only by EQC, GEQC, and GC
         *  When op1 and/or op2 are NULL, then left and right point to the input shares to be used
         *for this op_type.
         **/
        Predicate_B p = {EQ, NULL, NULL, 0, 0};

        // Equality Check of left and right cols and True or False is stored in res
        join_b(in1, in2, p, res);

        // reveal the result
        Data out[ROWS1 * ROWS2]; /** TODO: only rank0 needs to allocate **/
        open_b_array(res, ROWS1 * ROWS2, out);

        // Store indices
        std::vector<int> t1_index;
        std::vector<int> t2_index;
        for (int i = 0; i < ROWS1 * ROWS2; i++) {
            if (out[i] == 1) {
                int t1 = i / ROWS2;
                int t2 = i % ROWS2;
                t1_index.push_back(t1);
                t2_index.push_back(t2);
            }
        }

        // Send Size of t2 index
        int size_to_receive = t2_index.size();
        MPI_Send(&size_to_receive, 1, MPI_INT, 1, RESULT_TAG, MPI_COMM_WORLD);
        MPI_Send(t2_index.data(), size_to_receive, MPI_INT, 1, RESULT_TAG, MPI_COMM_WORLD);

        // JSON object to hold the results
        jsoncons::json output_json = jsoncons::json::array();

        // std::cout << "/// Joined Table ///" << std::endl;
        for (int i = 0; i < size_to_receive; i++) {
            jsoncons::json entry = jsoncons::json::object();
            
            // Index/Key Val
            int t1 = t1_index[i];
            long long index_val = js1[t1][0].as<int>();
            entry["index_val"] = index_val;
            // std::cout << "[" << index_val;

            // Build Own Table
            std::vector<int> send_vals(COLS1-1);
            for(int j = 1; j < COLS1; j++){
                int curr_val = js1[t1][j].as<int>();
                send_vals[j-1] = curr_val;
                entry["own_val" + std::to_string(j)] = curr_val;
            }

            // Their Table from P2
            std::vector<int> rec_vals(COLS2-1);
            MPI_Recv(rec_vals.data(), rec_vals.size(), MPI_LONG_LONG, 1, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            
            // Send to P2
            MPI_Send(send_vals.data(), send_vals.size(), MPI_LONG_LONG, 1, RESULT_TAG, MPI_COMM_WORLD);
            for (size_t i = 0; i < rec_vals.size(); ++i) {
               int curr_val = rec_vals[i];
               entry["their_val" + std::to_string(i)] = curr_val;
            }

            // Add the entry to the output JSON array
            output_json.push_back(entry);
        }
        upload_to_s3(0, output_json, "output.json");
    } else if (rank == 1) {  // P2
        std::string filename = argv[2];
        if (filename.substr(filename.find_last_of(".") + 1) != "csv") {
            std::cerr << "Error: The file '" << filename << "' is not a CSV file. Please provide a .csv file." << std::endl;
            return 1;
        }
        download_from_s3(rank + 1, filename);
        init_sharing();
        std::string csv_file = "./../" + filename;
        std::ifstream is2(csv_file);
        ojson js2 = csv::decode_csv<ojson>(is2,options);
        ROWS2 = static_cast<int>(js2.size());
        COLS2 = static_cast<int>(js2[0].size());

        //////// Receive ROWS1 and COLS1 from P1
        MPI_Recv(&ROWS1, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&COLS1, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //////// Send ROWS2 and COLS2 to P1 and 3
        MPI_Send(&ROWS2, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&COLS2, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&ROWS2, 1, MPI_INT, 2, INIT_TAG, MPI_COMM_WORLD);
        MPI_Send(&COLS2, 1, MPI_INT, 2, INIT_TAG, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);
        BShare r1s1[ROWS1][COLS1], r1s2[ROWS1][COLS1], r1s3[ROWS1][COLS1], r2s1[ROWS2][COLS2], r2s2[ROWS2][COLS2], r2s3[ROWS2][COLS2];
        Data r2[ROWS2][COLS2];

        for (int i = 0; i < ROWS2; ++i) {
            r2[i][0] = js2[i][0].as<int>();
            r2[i][1] = js2[i][1].as<int>();
        }

        // generate r2 shares
        for (int i = 0; i < ROWS2; i++) {
            for (int j = 0; j < COLS2; j++) {
                generate_bool_share(r2[i][j], &r2s1[i][j], &r2s2[i][j], &r2s3[i][j]);
            }
        }

        // Receive from P1
        MPI_Recv(&r1s1[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r1s2[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Send shares to P1
        MPI_Send(&r2s1[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD);
        MPI_Send(&r2s2[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD);

        // Send shares to P3
        MPI_Send(&r2s3[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
        MPI_Send(&r2s1[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 2, SHARE_TAG, MPI_COMM_WORLD);
        MPI_Barrier(MPI_COMM_WORLD);

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
        // exchange seeds
        /** sending random bytes to successor(i.e., to party 2 if you are party 1), called
        seed_local and receiving the same from predecessor (i.e., party 3 if you are party 1),
        called seed_remote
          * Using those seed bytes, set local_state and remote_state with random values
        **/
        exchange_rsz_seeds(succ, pred);

        BShare res[ROWS1 * ROWS2];  // join result to be stored in res 5 x 4 size

        // allocate BShare tables
        // BShareTable: {int ownerId; int partyId; int numRows; int numCols; int relationId; Share
        // **content (2d array) } It sets 'content' memory space for actual data separately inside
        // allocate_bool ... func
        BShareTable in1 = {-1, rank, ROWS1, 2 * COLS1, 1}, in2 = {-1, rank, ROWS2, 2 * COLS2, 2};
        allocate_bool_shares_table(&in1);  // Passing address of in1 and sets pointers and actual
                                           // values in contiguous memory address
        allocate_bool_shares_table(&in2);

        // copy shares into the BShareTables
        // store original rand val s1 of own party and same rand val from party 1 in s2
        for (int i = 0; i < ROWS1; i++) {
            in1.content[i][0] = r1s1[i][0];
            in1.content[i][1] = r1s2[i][0];
            in1.content[i][2] = r1s1[i][1];
            in1.content[i][3] = r1s2[i][1];
        }
        // relation 2
        for (int i = 0; i < ROWS2; i++) {
            in2.content[i][0] = r2s1[i][0];
            in2.content[i][1] = r2s2[i][0];
            in2.content[i][2] = r2s1[i][1];
            in2.content[i][3] = r2s2[i][1];
        }

        /** struct predicate_b {OP_TYPE operation; struct predicate_b *op1; struct predicate_b *op2;
         *    int leftcol; int rightcol; BShare cs1; BShare cs2;   // Used only by EQC, GEQC, and GC
         *  When op1 and/or op2 are NULL, then left and right point to the input shares to be used
         *for this op_type.
         **/
        Predicate_B p = {EQ, NULL, NULL, 0, 0};

        // Equality Check of left and right cols and True or False is stored in res
        join_b(in1, in2, p, res);

        // reveal the result
        Data out[ROWS1 * ROWS2]; /** TODO: only rank0 needs to allocate **/
        open_b_array(res, ROWS1 * ROWS2, out);

        // Receive size of t2 index
        int size_to_send;
        MPI_Recv(&size_to_send, 1, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive the actual t2_index
        std::vector<int> t2_index(size_to_send);
        MPI_Recv(t2_index.data(), size_to_send, MPI_INT, 0, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // JSON object to hold the results
        jsoncons::json output_json = jsoncons::json::array();

        std::cout << "/// Joined Table ///" << std::endl;
        for (int i = 0; i < size_to_send; i++) {
            jsoncons::json entry = jsoncons::json::object();

            // Index/Key Val
            int t2 = t2_index[i];
            long long index_val = js2[t2][0].as<int>();
            entry["index_val"] = index_val;
            std::cout << "[" << index_val;

            // Build Own Table
            std::vector<int> send_vals(COLS2-1);
            for(int j = 1; j < COLS2; j++){
                int curr_val = js2[t2][j].as<int>();
                send_vals[j-1] = curr_val;
                entry["own_val" + std::to_string(j)] = curr_val;
                std::cout << ", " << curr_val;
            }
            // Send to P1
            MPI_Send(send_vals.data(), send_vals.size(), MPI_LONG_LONG, 0, RESULT_TAG, MPI_COMM_WORLD);

            // Their table from P1
            std::vector<int> rec_vals(COLS1-1);
            MPI_Recv(rec_vals.data(), rec_vals.size(), MPI_LONG_LONG, 0, RESULT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            for (size_t i = 0; i < rec_vals.size(); i++){
                int curr_val = rec_vals[i];
                entry["their_val" + std::to_string(i)] = curr_val;
                std::cout << ", " << curr_val;
            }
            std::cout << "]" << std::endl;
            output_json.push_back(entry);
        }
        upload_to_s3(1, output_json, "output.json");
    } else {  // P3

        //////// Receive ROWS1 and COLS1 from P1
        MPI_Recv(&ROWS1, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&COLS1, 1, MPI_INT, 0, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        //////// Receive ROWS2 and COLS2 from P2
        MPI_Recv(&ROWS2, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&COLS2, 1, MPI_INT, 1, INIT_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);

        BShare r1s1[ROWS1][COLS1], r1s2[ROWS1][COLS1], r1s3[ROWS1][COLS1], r2s1[ROWS2][COLS2], r2s2[ROWS2][COLS2], r2s3[ROWS2][COLS2];

        Data r2[ROWS2][COLS2];
        // Receive from P1
        MPI_Recv(&r1s1[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r1s2[0][0], ROWS1 * COLS1, MPI_LONG_LONG, 0, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);

        // Receive from P2
        MPI_Recv(&r2s1[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(&r2s2[0][0], ROWS2 * COLS2, MPI_LONG_LONG, 1, SHARE_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Barrier(MPI_COMM_WORLD);
        // exchange seeds
        /** sending random bytes to successor(i.e., to party 2 if you are party 1), called
        seed_local and receiving the same from predecessor (i.e., party 3 if you are party 1),
        called seed_remote
          * Using those seed bytes, set local_state and remote_state with random values
        **/
        exchange_rsz_seeds(succ, pred);

        BShare res[ROWS1 * ROWS2];  // join result to be stored in res 5 x 4 size

        // allocate BShare tables
        // BShareTable: {int ownerId; int partyId; int numRows; int numCols; int relationId; Share
        // **content (2d array) } It sets 'content' memory space for actual data separately inside
        // allocate_bool ... func
        BShareTable in1 = {-1, rank, ROWS1, 2 * COLS1, 1}, in2 = {-1, rank, ROWS2, 2 * COLS2, 2};
        allocate_bool_shares_table(&in1);  // Passing address of in1 and sets pointers and actual
                                           // values in contiguous memory address
        allocate_bool_shares_table(&in2);

        // copy shares into the BShareTables
        // store original rand val s1 of own party and same rand val from party 1 in s2
        for (int i = 0; i < ROWS1; i++) {
            in1.content[i][0] = r1s1[i][0];
            in1.content[i][1] = r1s2[i][0];
            in1.content[i][2] = r1s1[i][1];
            in1.content[i][3] = r1s2[i][1];
        }
        // relation 2
        for (int i = 0; i < ROWS2; i++) {
            in2.content[i][0] = r2s1[i][0];
            in2.content[i][1] = r2s2[i][0];
            in2.content[i][2] = r2s1[i][1];
            in2.content[i][3] = r2s2[i][1];
        }

        /** struct predicate_b {OP_TYPE operation; struct predicate_b *op1; struct predicate_b *op2;
         *    int leftcol; int rightcol; BShare cs1; BShare cs2;   // Used only by EQC, GEQC, and GC
         *  When op1 and/or op2 are NULL, then left and right point to the input shares to be used
         *for this op_type.
         **/
        Predicate_B p = {EQ, NULL, NULL, 0, 0};

        // Equality Check of left and right cols and True or False is stored in res
        join_b(in1, in2, p, res);

        // reveal the result
        Data out[ROWS1 * ROWS2]; /** TODO: only rank0 needs to allocate **/
        open_b_array(res, ROWS1 * ROWS2, out);
    }

    // tear down communication
    MPI_Finalize();
    return 0;
}