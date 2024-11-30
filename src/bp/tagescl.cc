
Conversation opened. 11 messages. All messages read.

Skip to content
Using UC Santa Cruz Mail with screen readers
2 of 10,085
Lab3 - Project Check-In
Inbox
Xiangyue Huang
	
	Nov 23, 2024, 3:56 PM (6 days ago)
Hi Aidan, According to the doc, we need to create a git repository link referencing the current source-code implementation within Scarab. In addition, I think w
Aidan Gray
	
	Nov 24, 2024, 1:22 PM (5 days ago)
I'm really sorry I missed your email, if you don't already have a git repository I can make one. I'm also planning on looking over the source code of bp, and I'
Xiangyue Huang
	
	Nov 24, 2024, 1:34 PM (5 days ago)
It's okay, could you please make a git repo as the workspace? In addition, don't hesitate to email me for any progress. Best regards, Xiangyue
Aidan Gray <aitgray@ucsc.edu>
	
Nov 24, 2024, 2:07 PM (5 days ago)
	
to Xiangyue
Ok, here is the git repo: https://github.com/Aitgray/220_lab3
Aidan Gray
	
	Nov 24, 2024, 2:33 PM (5 days ago)
I just pushed some changes to bp.h and bp_table.def so we can start working on the bp.c file. I added the new data structures and helper functions to the header
Aidan Gray
	
	Nov 24, 2024, 5:20 PM (5 days ago)
I pushed another change, I started the implementation of the perceptrons in bp.c, but the functions are not complete yet. Additionally I made a few other small
Xiangyue Huang
	
	Nov 24, 2024, 5:23 PM (5 days ago)
Thanks for your work, I will read your modifications tonight and try to provide some feedback.
Xiangyue Huang
	
AttachmentsNov 25, 2024, 12:12 AM (4 days ago)
	
to me
According to the paper, the threshold should be set to zero, the branch is predicted not taken when dot_product is negative, or taken otherwise.

In the perceptron_update process, I think we may need to update global_hist first, and then update weights. The paper says that the weights should be updated after the outcome is known, so we may need to update weights when the prediction is correct.
 One attachment  •  Scanned by Gmail
Xiangyue Huang
	
Nov 27, 2024, 10:12 PM (2 days ago)
	
to me
I find that we need to implement a bp code like the tagescl.cc, I will try to implement this tomorrow.
Aidan Gray <aitgray@ucsc.edu>
	
Nov 27, 2024, 10:49 PM (2 days ago)
	
to Xiangyue
Ok, thank you for the update. I was traveling most of yesterday and today, and I’ll be celebrating Thanksgiving tomorrow so I’ll implement the new threshold requirements on Friday. 
>

> On Nov 27, 2024, at 10:12 PM, Xiangyue Huang <hxiangyu@ucsc.edu> wrote:
Xiangyue Huang
	
AttachmentsNov 28, 2024, 4:46 PM (1 day ago)
	
to me
Hi Aidan,
Happy Thanksgiving! I have implemented the dynamic branch prediction into scarab, just need to change the tagescl.cc file, and change the --bp_mech           tagescl line in the PARAMS.sunny_cove. I attached the code below. I only updated the initialization, predict and update function.
In addition, here is the result of benchmark 500, it doesn't show improvement compared to the baseline (tagescl), but I guess it's okay.
baseline:
hit: 1956985
miss: 486

dynamic branch:
hit: 1937722
miss: 19460

Please email me of your thoughts.

Best regards,
Xiangyue
 One attachment  •  Scanned by Gmail
	
Thanks, I will take a look.
Looks good, thanks!
Looks good, please proceed.

/* Copyright 2020 HPS/SAFARI Research Groups
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "tagescl.h"

#include <iostream>
#include <memory>

extern "C" {
#include "bp.param.h"
#include "core.param.h"
#include "globals/assert.h"
#include "table_info.h"
}

#include "bp/template_lib/tagescl.h"

namespace {
// A vector of TAGE-SC-L tables. One table per core.
std::vector<std::unique_ptr<Tage_SC_L_Base>> tagescl_predictors;

struct Perceptron {
  std::vector<int32_t> weights; // Vector to store weights

  // Constructor to initialize weights with 32 elements set to 0
  Perceptron() : weights(32, 0) { 
    // All weights initialized to zero
  }
};

// Define the DPbase class
class DPbase {
public:
  std::vector<std::unique_ptr<Perceptron>> perceptrons; // Store Perceptron objects
  int perceptron_num;
  const int MAX_WEIGHT = 50;
  const int MIN_WEIGHT = -50;

  DPbase(int size) {
    perceptron_num=size;
    for (int i = 0; i < size; ++i) {
      perceptrons.push_back(std::make_unique<Perceptron>());
    }
  }

  uns8 get_prediction(uns32 global_hist, Addr branch_address) {
    int index = branch_address % perceptron_num; // Hash the branch address to get the index
    Perceptron* p = perceptrons[index].get();      // Get the perceptron at the index
    int dot_product = 0;

    for (int i = 0; i < 32; i++) {
      int bit = (global_hist >> i) & 1;            // Get the i-th bit of the global history
      dot_product += (bit ? 1 : -1) * p->weights[i]; // Multiply history bit with perceptron weight
    }
    return dot_product >= 0 ? TAKEN : NOT_TAKEN;   // Return prediction
  }
  
void update(uns32 global_hist,Addr branch_address,uns8 outcome){
  int index = branch_address % perceptron_num;
  Perceptron* p = perceptrons[index].get();
  // update weights
    for (int i = 0; i < 32; i++) {
      int bit = (global_hist >> i) & 1;
      p->weights[i] += (bit ? 1 : -1) * (outcome ? 1 : -1);
      // clamp weights to some arbitrary range, MAX and MIN WEIGHTS are placeholders, this will NOT compile
      if (p->weights[i] > MAX_WEIGHT) p->weights[i] = MAX_WEIGHT;
      if (p->weights[i] < MIN_WEIGHT) p->weights[i] = MIN_WEIGHT;
    }
}

  // Destructor is automatically managed due to smart pointers
  ~DPbase() = default;
};

// Global vector to store DPbase objects
std::vector<std::unique_ptr<DPbase>> dp_predictors;


// Helper function for producing a Branch_Type struct.
Branch_Type get_branch_type(uns proc_id, Cf_Type cf_type) {
  Branch_Type br_type;
  switch(cf_type) {
    case CF_BR:
    case CF_CALL:
      br_type.is_conditional = false;
      br_type.is_indirect    = false;
      break;
    case CF_CBR:
      br_type.is_conditional = true;
      br_type.is_indirect    = false;
      break;
    case CF_IBR:
    case CF_ICALL:
    case CF_ICO:
    case CF_RET:
    case CF_SYS:
      br_type.is_conditional = false;
      br_type.is_indirect    = true;
      break;
    default:
      // Should never see non-control flow instructions or invalid CF
      // types in the branch predictor.
      ASSERT(proc_id, false);
      break;
  }
  return br_type;
}
}  // end of anonymous namespace

void bp_tagescl_init() {
  if(tagescl_predictors.size() == 0) {
    tagescl_predictors.reserve(NUM_CORES);
    for(uns i = 0; i < NUM_CORES; ++i) {
      if(BP_MECH == TAGESCL_BP) {
        tagescl_predictors.push_back(
          std::make_unique<Tage_SC_L<TAGE_SC_L_CONFIG_64KB>>(NODE_TABLE_SIZE));
      } else {
        tagescl_predictors.push_back(
          std::make_unique<Tage_SC_L<TAGE_SC_L_CONFIG_80KB>>(NODE_TABLE_SIZE));
      }
    }
  }
  ASSERTM(0, tagescl_predictors.size() == NUM_CORES,
          "tagescl_predictors not initialized correctly");

  if(dp_predictors.size() == 0) {
    dp_predictors.reserve(NUM_CORES);
    for(uns i = 0; i < NUM_CORES; ++i) {
      if(BP_MECH == TAGESCL_BP) {
        dp_predictors.push_back(
          std::make_unique<DPbase>(NODE_TABLE_SIZE));
      } else {
        dp_predictors.push_back(
          std::make_unique<DPbase>(NODE_TABLE_SIZE));
      }
    }
  }
  ASSERTM(0, dp_predictors.size() == NUM_CORES,
          "dp_predictors not initialized correctly");
   
}

void bp_tagescl_timestamp(Op* op) {
  uns proc_id = op->proc_id;
  op->recovery_info.branch_id =
    tagescl_predictors.at(proc_id)->get_new_branch_id();
}

uns8 bp_tagescl_pred(Op* op) {
  uns proc_id = op->proc_id;
  //return tagescl_predictors.at(proc_id)->get_prediction(
   // op->recovery_info.branch_id, op->inst_info->addr);
return dp_predictors.at(proc_id)->get_prediction(op->oracle_info.pred_global_hist, op->inst_info->addr);
}

void bp_tagescl_spec_update(Op* op) {
  uns proc_id = op->proc_id;
  tagescl_predictors.at(proc_id)->update_speculative_state(
    op->recovery_info.branch_id, op->inst_info->addr,
    get_branch_type(proc_id, op->table_info->cf_type), op->oracle_info.pred,
    op->oracle_info.target);
}

void bp_tagescl_update(Op* op) {
  uns proc_id = op->proc_id;
  //tagescl_predictors.at(proc_id)->commit_state(
  //  op->recovery_info.branch_id, op->inst_info->addr,
  //  get_branch_type(proc_id, op->table_info->cf_type), op->oracle_info.dir);
  dp_predictors.at(proc_id)->update(op->oracle_info.pred_global_hist,op->inst_info->addr,op->oracle_info.dir);
  
}

void bp_tagescl_retire(Op* op) {
  uns proc_id = op->proc_id;
  tagescl_predictors.at(proc_id)->commit_state_at_retire(
    op->recovery_info.branch_id, op->inst_info->addr,
    get_branch_type(proc_id, op->table_info->cf_type), op->oracle_info.dir,
    op->oracle_info.target);
}

void bp_tagescl_recover(Recovery_Info* recovery_info) {
  uns proc_id = recovery_info->proc_id;
  tagescl_predictors.at(proc_id)->flush_branch_and_repair_state(
    recovery_info->branch_id, recovery_info->PC,
    get_branch_type(proc_id, recovery_info->cf_type), recovery_info->new_dir,
    recovery_info->branchTarget);
}

uns8 bp_tagescl_full(uns proc_id) {
    return tagescl_predictors.at(proc_id)->is_full();
}

tagescl.cc
Displaying tagescl.cc.
