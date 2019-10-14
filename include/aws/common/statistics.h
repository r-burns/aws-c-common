#ifndef AWS_COMMON_STATISTICS_H
#define AWS_COMMON_STATISTICS_H

/*
 * Copyright 2010-2019 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * expressaws_crt_statistics_base or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

#include <aws/common/common.h>
#include <aws/common/package.h>

#include <aws/common/stdint.h>

struct aws_array_list;

typedef uint32_t aws_crt_statistics_category_t;

#define AWS_CRT_STATISTICS_CATEGORY_STRIDE_BITS 8
#define AWS_CRT_STATISTICS_CATEGORY_STRIDE (1U << AWS_CRT_STATISTICS_CATEGORY_STRIDE_BITS)
#define AWS_CRT_STATISTICS_CATEGORY_BEGIN_RANGE(x) ((x)*AWS_CRT_STATISTICS_CATEGORY_STRIDE)
#define AWS_CRT_STATISTICS_CATEGORY_END_RANGE(x) (((x) + 1) * AWS_CRT_STATISTICS_CATEGORY_STRIDE - 1)

/**
 * The common-specific range of the aws_crt_statistics_category cross-library enum.
 *
 * This enum functions as an RTTI value that lets statistics handler's interpret (via cast) a
 * specific statistics structure if the RTTI value is understood.
 *
 * Common doesn't have any statistics structures presently, so its range is essentially empty.
 *
 */
enum aws_crt_common_statistics_category {
    AWSCRT_STAT_CAT_INVALID = AWS_CRT_STATISTICS_CATEGORY_BEGIN_RANGE(AWS_C_COMMON_PACKAGE_ID)
};

/**
 * Pattern-struct that functions as a base "class" for all statistics structures.  To conform
 * to the pattern, a statistics structure must have its first member be the category.  In that
 * case it becomes "safe" to cast from aws_crt_statistics_base to the specific statistics structure
 * based on the category value.
 */
struct aws_crt_statistics_base {
    aws_crt_statistics_category_t category;
};

/**
 * The start and end time, in milliseconds-since-epoch, that a set of statistics was gathered over.
 */
struct aws_crt_statistics_sample_interval {
    uint64_t begin_time_ms;
    uint64_t end_time_ms;
};

struct aws_crt_statistics_handler;

/*
 * Statistics intake function.  The array_list is a list of pointers to aws_crt_statistics_base "derived" (via
 * pattern) objects.  The handler should iterate the list and downcast elements whose RTTI category it understands,
 * while skipping those it does not understand.
 */
typedef void(aws_crt_statistics_handler_process_statistics_fn)(
    struct aws_crt_statistics_handler *,
    struct aws_crt_statistics_sample_interval *,
    struct aws_array_list *);

/*
 * Cleans up all additional resources, including the impl itself if appropriate, related to a specific statistics
 * handler implementation.
 */
typedef void(aws_crt_statistics_handler_cleanup_fn)(struct aws_crt_statistics_handler *);

/*
 * The period, in milliseconds, that the handler would like to be informed of statistics.  Statistics generators are
 * not required to honor this value, but should if able.
 */
typedef uint64_t(aws_crt_statistics_handler_get_report_interval_ms_fn)(struct aws_crt_statistics_handler *);

/**
 * Vtable for functions that all statistics handlers must implement
 */
struct aws_crt_statistics_handler_vtable {
    aws_crt_statistics_handler_process_statistics_fn *process_statistics;
    aws_crt_statistics_handler_cleanup_fn *cleanup;
    aws_crt_statistics_handler_get_report_interval_ms_fn *get_report_interval_ms;
};

/**
 * Base structure for all statistics handler implementations.
 *
 * A statistics handler is an object that listens to a stream of polymorphic (via the category RTTI enum) statistics
 * structures emitted from some arbitrary source.  In the initial implementation, statistics handlers are primarily
 * attached to channels, where they monitor IO throughput and state data (from channel handlers) to determine a
 * connection's health.
 *
 * Statistics handlers are a generalization of the timeout and bandwidth filters that are often associated with
 * SDK network connections.  Configurable, default implementations are defined at the protocol level (http, etc...)
 * where they can be attached at connection (channel) creation time.
 */
struct aws_crt_statistics_handler {
    struct aws_crt_statistics_handler_vtable *vtable;
    struct aws_allocator *allocator;
    void *impl;
};

AWS_EXTERN_C_BEGIN

/**
 * completely destroys a statistics handler.  The handler's cleanup function must clean up the impl portion completely
 * (including its allocation, if done separately).
 */
AWS_COMMON_API
void aws_crt_statistics_handler_destroy(struct aws_crt_statistics_handler *handler);

AWS_EXTERN_C_END

#endif /* AWS_COMMON_STATISTICS_H */