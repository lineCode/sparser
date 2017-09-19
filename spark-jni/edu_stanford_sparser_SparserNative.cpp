#include "edu_stanford_sparser_SparserNative.h"
#include <jni.h>
#include <stdlib.h>
#include <iostream>
#include "bench_json.h"
#include "common.h"
#include "rapidjson/document.h"
#include "rapidjson/error/en.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"
#include "sparser.h"

using namespace rapidjson;

// Performs a parse of the query using RapidJSON. Returns true if all the
// predicates match.
int parse_putin_russia(const char *line, void *data) {
    Document d;
    d.Parse(line);
    if (d.HasParseError()) {
        fprintf(stderr, "\nError(offset %u): %s\n",
                (unsigned)d.GetErrorOffset(),
                GetParseError_En(d.GetParseError()));
        fprintf(stderr, "Error line: %s", line);
        return false;
    }

    Value::ConstMemberIterator itr = d.FindMember("text");
    if (itr == d.MemberEnd()) {
        // The field wasn't found.
        return false;
    }
    if (strstr(itr->value.GetString(), "Putin") == NULL) {
        return false;
    }

    if (strstr(itr->value.GetString(), "Russia") == NULL) {
        return false;
    }

    return true;
}

// 2. Save projected fields instead of row indices
JNIEXPORT jlong JNICALL Java_edu_stanford_sparser_SparserNative_parse(
    JNIEnv *env, jobject obj, jstring filename_java, jlong buffer_addr_java,
    jlong start_java, jlong length_java, jlong record_size, jlong max_records) {
    bench_timer_t start = time_start();
    // Step 1: Convert the Java String (jstring) into C string (char*)
    char *filename_c = (char *)env->GetStringUTFChars(filename_java, NULL);
    if (NULL == filename_c) return 0;
    printf("In C, the string is: %s\n", filename_c);

    const char *predicates[] = {
        "Putin", "Russia",
    };
    const long num_records_parsed = bench_sparser_hdfs(filename_c, start_java, length_java, predicates,
        2, parse_putin_russia, (void *)buffer_addr_java);
    assert(num_records_parsed <= max_records);
    env->ReleaseStringUTFChars(filename_java, filename_c); // release resources

    const double time = time_stop(start);
    printf("Total Time in C++: %f\n", time);
    return num_records_parsed;
}
