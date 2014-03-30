/**
 INTEL CONFIDENTIAL

 Copyright (C) 2007 by Intel Corporation.
 All Rights Reserved.

 The source code contained or described herein and all documents
 related to the source code ("Material") are owned by Intel Corporation
 or its suppliers or licensors. Title to the Material remains with Intel
 Corporation or its suppliers and licensors. The Material contains trade
 secrets and proprietary and confidential information of Intel or its
 suppliers and licensors. The Material is protected by worldwide copyright
 and trade secret laws and treaty provisions. No part of the Material may
 be used, copied, reproduced, modified, published, uploaded, posted,
 transmitted, distributed, or disclosed in any way without Intel's prior
 express written permission.

 No License under any patent, copyright, trade secret or other intellectual
 property right is granted to or conferred upon you by disclosure or
 delivery of the Materials, either expressly, by implication, inducement,
 estoppel or otherwise. Any license under such intellectual property rights
 must be express and approved by Intel in writing.

 Description:
 Helloworld for MYO shared programming model
 In this example, one global shared data(*buffer) is declared, and inited
 at myoiUserInit(). After runtime init(myoiLibInit), both host and LRB can
 have the buffer pointed to the same address.
*/

#include <fstream>
#include "common.h"
#include "check.h"
#include "omp.h"

#define NTHREADS 2

class APITESTS: public :: testing::Test
{
protected:
  size_t num_bandwidth;
  int *bandwidth;

  APITESTS()
  {}

  void SetUp()
  {
    const char *node_bandwidth_path = "/etc/numakind/node-bandwidth";
    std::ifstream nbw_file;

    nbw_file.open(node_bandwidth_path, std::ifstream::binary);
    nbw_file.seekg(0, nbw_file.end);
    num_bandwidth = nbw_file.tellg()/sizeof(int);
    nbw_file.seekg(0, nbw_file.beg);
    bandwidth = new int[num_bandwidth];
    nbw_file.read((char *)bandwidth, num_bandwidth*sizeof(int));
    nbw_file.close();
  }

  void TearDown()
  {
    delete[] bandwidth;
  }

  int check_page_hbw(void *ptr, size_t size)
  {
    Check checkObj;
    memset(ptr, 0, size);
    return checkObj.check_page_hbw(num_bandwidth, bandwidth, ptr, size);
  }

};


TEST_F(APITESTS,HbwExistsTest){

  int ret = HBW_SUCCESS;
  ret = HBW_IsHBWAvailable();
  ASSERT_EQ(1,ret);

}

TEST_F(APITESTS,HbwPolicy){
  ASSERT_EQ(1, HBW_getpolicy());
  HBW_setpolicy(2);
  ASSERT_EQ(2, HBW_getpolicy());
}


TEST_F(APITESTS, HbwMalloc2GB){
  char *ptr = NULL;
  int ret = HBW_SUCCESS;
  size_t size = (size_t)(2048*MB);
  ptr = (char *) HBW_malloc(size);
  if (NULL == ptr){
    ret = HBW_ERROR;
  }
  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwMalloc2KB){
  char *ptr = NULL;
  int ret = HBW_SUCCESS;
  size_t size = (size_t)(2*KB);
  ptr = (char *) HBW_malloc(size);
  if (NULL == ptr){
    ret = HBW_ERROR;
  }
  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwCalloc2GB){
  char *ptr = NULL;
  int ret = HBW_SUCCESS;
  size_t size = (size_t)(2*GB);
  ptr = (char *) HBW_calloc(size,1);
  if (NULL == ptr){
    ret = HBW_ERROR;
  }
  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwRealloc2GB){
  char *ptr = NULL;
  int ret = HBW_SUCCESS;
  size_t size = (size_t)(2*GB);
  ptr = (char *) HBW_realloc(ptr, size);

  if (NULL == ptr){
    ret = HBW_ERROR;
  }

  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }
  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwAllocateMemAlign2GB){

  void *ptr = NULL;
  int ret = HBW_SUCCESS, fret=0;
  size_t size = (size_t)(2*GB);
  size_t align = 32;

  fret = HBW_allocate_memalign(&ptr,align,size);

  if (fret != HBW_SUCCESS
      || ((size_t)ptr%align != 0)
      || (NULL == ptr)){
    ret = HBW_ERROR;
    goto exit;
  }

  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

 exit:
  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwAllocateMemAlignPsize4K){

  void *ptr = NULL;
  int ret = HBW_SUCCESS, fret=0;
  size_t size = (size_t)(16*MB);
  size_t align = 4*KB;
  hbw_pagesize_t psize = HBW_PAGESIZE_4KB;

  fret = HBW_allocate_memalign_psize(&ptr,align,size,psize);

  if (fret != HBW_SUCCESS
      || (NULL == ptr)){
    ret = HBW_ERROR;
    goto exit;
  }

  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

 exit:
  ASSERT_EQ(HBW_SUCCESS, ret);
}

TEST_F(APITESTS, HbwAllocateMemAlignPsize2M){

  void *ptr = NULL;
  int ret = HBW_SUCCESS, fret=0;
  size_t size = (size_t)(16*MB);
  size_t align = 2*MB;
  hbw_pagesize_t psize = HBW_PAGESIZE_2MB;

  fret = HBW_allocate_memalign_psize(&ptr,align,size,psize);

  if (fret != HBW_SUCCESS
      || (NULL == ptr)){
    ret = HBW_ERROR;
    goto exit;
  }

  /*Check that we got high bandwidth nodes*/
  ASSERT_EQ(HBW_SUCCESS, check_page_hbw(ptr, size));

  if (NULL != ptr){
    HBW_free(ptr);
  }

 exit:
  ASSERT_EQ(HBW_SUCCESS, ret);
}


