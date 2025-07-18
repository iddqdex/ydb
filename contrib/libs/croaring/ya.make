# Generated by devtools/yamaker from nixpkgs 24.05.

LIBRARY()

LICENSE(
    Apache-2.0 AND
    BSD-3-Clause AND
    MIT
)

LICENSE_TEXTS(.yandex_meta/licenses.list.txt)

VERSION(4.3.5)

ORIGINAL_SOURCE(https://github.com/RoaringBitmap/CRoaring/archive/v4.3.5.tar.gz)

ADDINCL(
    GLOBAL contrib/libs/croaring/include
)

NO_COMPILER_WARNINGS()

NO_RUNTIME()

SRCS(
    src/array_util.c
    src/art/art.c
    src/bitset.c
    src/bitset_util.c
    src/containers/array.c
    src/containers/bitset.c
    src/containers/containers.c
    src/containers/convert.c
    src/containers/mixed_andnot.c
    src/containers/mixed_equal.c
    src/containers/mixed_intersection.c
    src/containers/mixed_negation.c
    src/containers/mixed_subset.c
    src/containers/mixed_union.c
    src/containers/mixed_xor.c
    src/containers/run.c
    src/isadetection.c
    src/memory.c
    src/roaring.c
    src/roaring64.c
    src/roaring_array.c
    src/roaring_priority_queue.c
)

END()
