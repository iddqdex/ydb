PROGRAM()

SRCS(
    main.cpp
    pagination_data.cpp
    pagination.cpp
)

PEERDIR(
    library/cpp/getopt
    ydb/public/sdk/cpp/src/client/table
)

END()
