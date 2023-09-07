/*!
	\file
	\brief Модульные тесты.
	\authors Близнец Р.А.
	\version 0.0.0.1
	\date 03.02.2023
	\copyright (c) Copyright 2023, ООО "Глобал Ориент", Москва, Россия, http://www.glorient.ru/
*/

// #include <limits.h>
#include <cstring>
#include "unity.h"
#include "CJsonParser.h"
#include "CTrace.h"

#define countof(x) (sizeof(x)/sizeof(x[0]))

TEST_CASE("CJsonParser", "[utils]")
{
   const char* str1="{\"tx\":{\"freq\":312100000,\"sin\":\"on\",\"time\":5000}}";
   const char* str2="{\"init\":[10,0,11,31,31,96,32,59,33,2,71,8,72,26,34,8,38,83,40,16,44,28]}";
   const int ar1[]={10,0,11,31,31,96,32,59,33,2,71,8,72,26,34,8,38,83,40,16,44,28};
   uint32_t mem1=esp_get_free_heap_size();

   int t1,t2,x;
   int* data;
	std::string str;
   CJsonParser* parser=new CJsonParser();
   STARTTIMESHOT();
   t1=parser->parse(str1);
   if(t1 == 1)
   {
      if(parser->getObject(t1, "tx", t2))
      {
         if(parser->getInt(t2, "freq", x))
         {
            TEST_ASSERT_EQUAL_INT(312100000,x);
            if(parser->getString(t2, "sin", str)  && (str == "on"))
            {
            }
            else TEST_FAIL();
            if(parser->getInt(t2, "time", x))
            {
               TEST_ASSERT_EQUAL_INT(5000,x);
            }
            else TEST_FAIL();
         }
         else TEST_FAIL();
      }
   }
   else TEST_FAIL();
   STOPTIMESHOT("parse time");

   t1=parser->parse(str2);
   if(t1 == 1)
   {
      if(parser->getArrayInt(t1, "init", data, x))
      {
         TEST_ASSERT_EQUAL_INT(countof(ar1),x);
         TEST_ASSERT_EQUAL_INT_ARRAY(ar1,data,x);
         delete[] data;
      }
      else TEST_FAIL();
   }
   else TEST_FAIL();

   delete parser;

   uint32_t mem2=esp_get_free_heap_size();
   if(mem1 != mem2)
   {
      TRACE("memory leak",mem1-mem2,false);
      TRACE("start",mem1,false);
      TRACE("stop",mem2,false);
      TEST_FAIL_MESSAGE("memory leak");
   }
}

