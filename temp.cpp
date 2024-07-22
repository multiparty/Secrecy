#include "../include/core/comm.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "mpi.h"

void join_eq_b(BShareTable input1, BShareTable input2,
                        int leftcol, int rightcol, BShare result[]) {
  int i, j, k = 0;

  for (i = 0; i < input1.numRows; i++) {
    for (j = 0; j < input2.numRows; j++) {
      // generate rnums for the next equality 
      // basically compairing left cols of r1s1 and r1s2 pair or r2s1 and r2s2 pair
      result[k++] = eq_b(input1.content[i][leftcol], input1.content[i][leftcol + 1],
                         input2.content[j][rightcol], input2.content[j][rightcol + 1]);
    }
  }
}

BShare eq_b(BShare x1, BShare x2, BShare y1, BShare y2) {
    // compute bitwise x^y^1
    BShare lbs = x1 ^ y1 ^ ~((BShare) 0); // local share (XOR r1s1 and r2s1)
    BShare rbs = x2 ^ y2 ^ ~((BShare) 0); // remote share (XOR r1s2 and r2s2)
    int numbits = sizeof(BShare) * 8; // 8 * 8 bits as BShare size is 8
    int rounds = (int) log2(numbits); // Number of rounds for oblivious equality = 6
    BShare shifted_lbs, shifted_rbs;
    for (int l=0; l<rounds-1; l++) { // l = from 0 to 4 as rounds = 6 => rounds - 1 = 5
        // Compute first round l: l-th round (0 to 4)
        shift_greater(lbs, rbs, l, numbits, &shifted_lbs, &shifted_rbs);
        lbs = and_b(lbs, rbs, shifted_lbs, shifted_rbs, get_next_rb());
        rbs = exchange_shares(lbs); //lbs is sent to predecessor and receive rbs from successor
    }
    // Do final round without exchanging shares
    shift_greater(lbs, rbs, rounds-1, numbits, &shifted_lbs, &shifted_rbs);
    lbs = and_b(lbs, rbs, shifted_lbs, shifted_rbs, get_next_rb());
    // Return one share of the final result (LSB).
    // Note: We need to call exchange again before using it in a subsequent operation.
    return lbs & (BShare) 1; // 1 = equal, 0 = otherwise
}
// r: r-th rounds (0 to 4), len: numbits = 8
inline PRIVATE void shift_greater(const BShare x1, const BShare x2, int r, int len,
                                  BShare *shifted_1, BShare *shifted_2) {
    int part_size = ((int) 1 << r); // bit val with rth iteration of rounds being 1 = int(2 * r) 0, 2, 4, 6, 8
    int offset = (part_size >> 1), start; // Half the part_size (=r/2-th iteration) 0, 1, 2, 3, 4
    BShare b1, b2;
    *shifted_1 = x1; // Store local boolean share
    *shifted_2 = x2; // Store remote boolean share
    // Loop over numbits from idx-0
    for (int i=0; i<len; i+=part_size) {
        start = i + offset; // Shift up by 1/2 of rounds (0,1,2,3,4)
        b1 = get_bit(x1, start); // Getting bit val at i
        b2 = get_bit(x2, start);
        for (int j=start-1; j>=i; j--) { // This runs when round(r) >= 1 (r ranges 0 - 1)
            *shifted_1 = unset_bit(*shifted_1,j) | (b1 << j); // Work on 1 bit before b1
            *shifted_2 = unset_bit(*shifted_2,j) | (b2 << j);
        }
    }
}1

// Logical AND using boolean shares (assumes three parties)
BShare and_b(const BShare x1, const BShare x2, const BShare y1,
                    const BShare y2, const BShare rnum) {
  BShare z = (x1 & y1) ^ (x1 & y2) ^ (x2 & y1);
  // XOR random number
  z ^= rnum;
  return z;
}


// exchange boolean shares: this is blocking
BShare exchange_shares(BShare s1) {
  BShare s2;
  // send s1 to predecessor
  MPI_Send(&s1, 1, MPI_LONG_LONG, get_pred(), XCHANGE_MSG_TAG, MPI_COMM_WORLD);
  // receive remote seed from successor
  MPI_Recv(&s2, 1, MPI_LONG_LONG, get_succ(), XCHANGE_MSG_TAG,
                                            MPI_COMM_WORLD, MPI_STATUS_IGNORE);
  return s2;
}