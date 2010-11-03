// Author: Mohamed Aly <malaa@vision.caltech.edu>
// Date: October 6, 2010

#include "mex.h"

#include <string>
#include <cassert>
#include <cstring>
#include <climits>
#include "ccInvertedFile.hpp"
#include "mxData.hpp"
#include "mxCommon.hpp"

// //Matlab types
// #define TYPEOF_mxSINGLE_CLASS   float
// #define TYPEOF_mxDOUBLE_CLASS   double
// #define TYPEOF_mxINT8_CLASS     char
// #define TYPEOF_mxUINT8_CLASS    unsigned char
// #define TYPEOF_mxINT32_CLASS    int
// #define TYPEOF_mxUINT32_CLASS   unsigned int
// 
// 
// //creates a switch statement with fun in every statement using "var"
// //as the class ID. "fun" is another macro that takes the class ID.
// #define __SWITCH(var,fun)                               \
//   switch(var)                                           \
//   {                                                     \
//     case mxSINGLE_CLASS:  fun(mxSINGLE_CLASS)  break;   \
//     case mxDOUBLE_CLASS:  fun(mxDOUBLE_CLASS)  break;   \
//     case mxINT8_CLASS:    fun(mxINT8_CLASS)    break;   \
//     case mxUINT8_CLASS:   fun(mxUINT8_CLASS)   break;   \
//     case mxINT32_CLASS:   fun(mxINT32_CLASS)   break;   \
//     case mxUINT32_CLASS:  fun(mxUINT32_CLASS)  break;   \
//   }                                                     

//inputs
#define ivFileIn  prhs[0]
#define dataIn    prhs[1]
#define wtIn      prhs[2]
#define normIn    prhs[3]
#define distIn    prhs[4]
#define nretIn    prhs[5]
#define overlapIn prhs[6]
#define verboseIn prhs[7]
//outputs          
#define docsOut   plhs[0]
#define scoresOut plhs[1]
          
         

void mexFunction( int nlhs, mxArray *plhs[],
                  int nrhs, const mxArray *prhs[])
{
   
  if (nrhs!=8)	mexErrMsgTxt("Seven inputs required");

  //get the class of points
  mxClassID classID = mxGetClassID(dataIn);
  //if cell
  if (mxIsCell(dataIn) && mxGetNumberOfElements(dataIn)>0)
    for (uint i=0; i<mxGetNumberOfElements(dataIn); ++i)
    {
      mxArray* el = mxGetCell(dataIn, i);
      if (el != NULL)
      {
        classID = mxGetClassID(el);
        break;
      }
    }
  
  //number of documents
  uint ndocs = mxIsCell(dataIn) ? mxGetNumberOfElements(dataIn) : 1;
  
  //number of returns
  uint nret = 0;
  if (nrhs>=6)  nret = (uint) *mxGetPr(nretIn);  
  
  //overlap only?
  bool overlapOnly = true;
  if (nrhs>6) overlapOnly = (bool) *mxGetPr(overlapIn);  
  
  //verbose?
  bool verbose = (bool) *mxGetPr(verboseIn);  
  //get the weighting to use
  char s[500];
  mxGetString(wtIn, s, 500);
  string str = s;
  ivFile::Weight wt;
  if (str == "none")        wt = ivFile::WEIGHT_NONE;
  else if (str == "bin")    wt = ivFile::WEIGHT_BIN;
  else if (str == "tf")     wt = ivFile::WEIGHT_TF;
  else if (str == "tfidf")  wt = ivFile::WEIGHT_TFIDF;
  else                      mexErrMsgTxt("Unknown weighting function");
  
  //get the normalization to use
  mxGetString(normIn, s, 500);
  str = s;
  ivFile::Norm norm;
  if (str == "none")       norm = ivFile::NORM_NONE;
  else if (str == "l0")    norm = ivFile::NORM_L0;
  else if (str == "l1")    norm = ivFile::NORM_L1;
  else if (str == "l2")    norm = ivFile::NORM_L2;
  else                     mexErrMsgTxt("Unknown normalization function");
  
  //get the distance to use
  mxGetString(distIn, s, 500);
  str = s;
  ivFile::Dist dist;
  if (str == "l1")        dist = ivFile::DIST_L1;
  else if (str == "l2")   dist = ivFile::DIST_L2;
  else if (str == "ham")  dist = ivFile::DIST_HAM;
  else if (str == "kl")   dist = ivFile::DIST_KL;
  else if (str == "cos")  dist = ivFile::DIST_COS;
  else if (str == "jac")  dist = ivFile::DIST_JAC;
  else if (str == "hist-int")  dist = ivFile::DIST_HISTINT;
  else                    mexErrMsgTxt("Unknown distance function");

  //make the ivFile object  
  ivFile* ivfile = *(ivFile**)mxGetData(ivFileIn);
  //check if not passed in an object, then create a new one
  if (ivfile == NULL)
    mexErrMsgTxt("Empty inverted file input");
  
  //the return score list
  ivNodeLists scorelists;
  scorelists.resize(ndocs);
  
//   cout << "  searching!!";
//   mexPrintf("searching!!\n");

  //call function
#define __CASE(CLASS)                                                   \
    {                                                                     \
    /*get the data*/                                                      \
    Data<TYPEOF_##CLASS> data;                                            \
    /*set data*/                                                          \
    fillData(data, dataIn);                                               \
    /*insert*/                                                            \
    ivSearchFile(*ivfile, data, wt, norm, dist, overlapOnly, nret,        \
            scorelists, verbose);                                         \
    /*clear data*/                                                        \
    data.clear();                                                         \
    }
            
    __SWITCH(classID, __CASE)
    
    
  mxArray *scores, *docs;
    
  //if we get all documents, then update nret with the number of docmuents
  //if it's zero
  if (!overlapOnly && nret==0)
    nret = ivfile->docs.size();
    
  //allocate output
  if (nret==0)
  {
    scores = mxCreateCellMatrix(1, ndocs);
    docs = mxCreateCellMatrix(1, ndocs);
  }
  else
  {
    scores = mxCreateNumericMatrix(nret, ndocs, mxSINGLE_CLASS,mxREAL);
    docs = mxCreateNumericMatrix(nret, ndocs, mxUINT32_CLASS,mxREAL);      
  }
  uint32_t* pdocs;
  float* pscores;
 
  //now loop on number of documents
//  cout << ndocs << " " << nret << endl;
  for (uint d=0; d<ndocs; ++d)
  {
    //get size of this list
    uint nlist = scorelists[d].size();
//    cout << " " << nlist;
    
    //get pointer to docs and score
    if (nret==0)
    {
      //create and get pointer
      mxSetCell(docs, d, mxCreateNumericMatrix(1, nlist, mxUINT32_CLASS, mxREAL));
      pdocs = (uint32_t*) mxGetPr(mxGetCell(docs, d));
      mxSetCell(scores, d, mxCreateNumericMatrix(1, nlist, mxSINGLE_CLASS, mxREAL));
      pscores = (float*) mxGetPr(mxGetCell(scores, d));
    }
    else
    {
      pdocs = (uint32_t*)mxGetPr(docs) + nret*d;
      pscores = (float*)mxGetPr(scores) + nret*d;      
    }
    
    //now we have pointers, so put data
//    nret = nret==0 ? nlist : nret;    
    for (uint i=0; i<nlist && (nret==0 || i<nret); ++i)
    {
      pscores[i] = (scorelists[d])[i].val;
      pdocs[i]   = (scorelists[d])[i].id + 1;      
    }
  }
   
  docsOut = docs;
  if (nlhs>1)
    scoresOut = scores;
  
  //clear 
  scorelists.clear();
}

