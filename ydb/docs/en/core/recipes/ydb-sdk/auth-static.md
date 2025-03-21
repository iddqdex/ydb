<!-- markdownlint-disable blanks-around-fences -->

# Username and password based authentication

Below are examples of the code for authentication based on a username and token in different {{ ydb-short-name }} SDKs.

{% list tabs %}

- C++

  ```c++
  auto driverConfig = NYdb::TDriverConfig()
     .SetEndpoint(endpoint)
     .SetDatabase(database)
     .SetCredentialsProviderFactory(NYdb::CreateLoginCredentialsProviderFactory({
        .User = "user",
        .Password = "password",
     }));

  NYdb::TDriver driver(driverConfig);
  ```

- Go (native)

  You can pass the username and password in the connection string. For example:

  ```shell
  "grpcs://login:password@localohost:2135/local"
  ```

  You can also explicitly pass them using the `ydb.WithStaticCredentials` parameter:

  {% include [auth-static-with-native](../../../_includes/go/auth-static-with-native.md) %}

- Go (database/sql)

  You can pass the username and password in the connection string. For example:

  {% include [auth-static-database-sql](../../../_includes/go/auth-static-database-sql.md) %}

  You can also explicitly pass the username and password at driver initialization via a connector using the `ydb.WithStaticCredentials` parameter:

  {% include [auth-static-with-database-sql](../../../_includes/go/auth-static-with-database-sql.md) %}

- Java

  ```java
  public void work(String connectionString, String username, String password) {
      AuthProvider authProvider = new StaticCredentials(username, password);

      GrpcTransport transport = GrpcTransport.forConnectionString(connectionString)
              .withAuthProvider(authProvider)
              .build());

      QueryClient queryClient = QueryClient.newClient(transport).build();

      doWork(queryClient);

      queryClient.close();
      transport.close();
  }
  ```

- JDBC

  ```java
  public void work(String username, String password) {
      Properties props = new Properties();
      props.setProperty("username", username);
      props.setProperty("password", password);
      try (Connection connection = DriverManager.getConnection("jdbc:ydb:grpc://localhost:2136/local", props)) {
        doWork(connection);
      }

      // Username and password can be passed via the special method of DriverManager
      try (Connection connection = DriverManager.getConnection("jdbc:ydb:grpc://localhost:2136/local", username, password)) {
        doWork(connection);
      }
  }
  ```

- Node.js

  {% include [auth-static](../../_includes/nodejs/auth-static.md) %}

- Python

  {% include [auth-static](../../_includes/python/auth-static.md) %}

- Python (asyncio)

  {% include [auth-static](../../_includes/python/async/auth-static.md) %}

- C# (.NET)

  ```C#
  using Ydb.Sdk;
  using Ydb.Sdk.Auth;

  const string endpoint = "grpc://localhost:2136";
  const string database = "/local";

  var config = new DriverConfig(
      endpoint: endpoint, // Database endpoint, "grpcs://host:port"
      database: database, // Full database path
      credentials: new StaticCredentialsProvider(user, password)
  );

  await using var driver = await Driver.CreateInitialized(config);
  ```

- PHP

  ```php
  <?php

  use YdbPlatform\Ydb\Ydb;
  use YdbPlatform\Ydb\Auth\Implement\StaticAuthentication;

  $config = [

      // Database path
      'database'    => '/local',

      // Database endpoint
      'endpoint'    => 'localhost:2136',

      // Auto discovery (dedicated server only)
      'discovery'   => false,

      // IAM config
      'iam_config'  => [
          'insecure' => true,
          // 'root_cert_file' => './CA.pem', // Root CA file (uncomment for dedicated server)
      ],

      'credentials' => new StaticAuthentication($user, $password)
  ];

  $ydb = new Ydb($config);
  ```

{% endlist %}
