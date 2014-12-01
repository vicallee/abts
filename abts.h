/* Licensed to the Apache Software Foundation (ASF) under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The ASF licenses this file to You under the Apache License, Version 2.0
 * (the "License"); you may not use this file except in compliance with
 * the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * Modify by vical@126.com 2013/11/4
 *
 */
#ifndef ABTS_H
#define ABTS_H

#ifdef __cplusplus
extern "C" {
#endif

/* one unit corresponds to a suite */
typedef struct abts_suite abts_suite;
typedef abts_suite *(*abts_suite_fntype)(abts_suite *suite);

/* one interface corresponds to one or more case */
typedef struct abts_case abts_case;
typedef void (*abts_case_fntype)(abts_case *tc, void *arg);

abts_suite *abts_add_suite(abts_suite *suite, const char *suite_name);
void abts_run_test(abts_suite *ts, abts_case_fntype f, void *arg);

void abts_log_message(const char *fmt, ...);
void abts_not_impl(abts_case *tc, const char *message, int lineno);
void abts_assert(abts_case *tc, int condition, const char *message, int lineno);

/* Following several assertions interface is enough */
#define ABTS_LOG				abts_log_message
#define ABTS_NOT_IMPL(msg)	abts_not_impl(tc, msg, __LINE__)
#define ABTS_ASSERT(a, msg)	abts_assert(tc, (int)(a), msg, __LINE__)
#define ABTS_TRUE(a)			ABTS_ASSERT(a, "Condition is false, but expected true")
#define ABTS_FAIL(msg)			ABTS_ASSERT(0, msg)

/* fns is NULL terminated list of functions */
int abts_main(int argc, char *argv[], abts_suite_fntype fns[]);

unsigned abts_tick();

#ifdef __cplusplus
}
#endif

#endif /* ABTS_H */

