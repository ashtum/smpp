## What is the SMPP?

**SMPP** is an Asio-based implementation of the SMPP protocol for the transfer of short message data between External Short Messaging Entities (ESMEs), Routing Entities (REs), and Short Message Service Center (SMSC).

### Quick usage

You can start with examples in the [example directory](example).

#### Asio universal asynchronous model
This library is written using Asio's composed operation and conforms to Asio universal asynchronous model, 
which means it can be used with callbacks, coroutines, futures, Boost.Fiber and any other form of completion tokens.
```C++
// C++20's coroutines
auto sequence_number = co_await session.async_send(submit_sm);

// Callbacks
session.async_send(submit_sm, [](auto ec,auto sequence_number){});

// Futures
auto fut = co_await session.async_send(submit_sm, asio::use_future);
```

#### Client and Server use the same `smpp::session`
A client connects a TCP socket to the server and constructs a `smpp::session`:
```C++
auto socket = asio::ip::tcp::socket{ executor };

co_await socket.async_connect({ asio::ip::tcp::v4(), 2775 });

auto session = smpp::session{ std::move(socket) };
```

A server accepts a TCP socket and constructs a `smpp::session`:
```C++
auto acceptor = asio::ip::tcp::acceptor(executor, { asio::ip::tcp::v4(), 2775 });

for (;;)
{
  auto socket = co_await acceptor.async_accept();
  auto session = smpp::session{ std::move(socket) };
  //...
}
```

#### Overloads for sending requests and responses
Sending a request completes with a `sequence_number` which can be used to map the received responses on the arrival.
```C++
auto sequence_number = co_await session.async_send(submit_sm);
```

Sending a response needs the `sequence_number` and `command_status`.
```C++
co_await session.async_send(submit_sm_resp, sequence_number, smpp::command_status::rok);
```

#### Enquire_link operation is handled by `smpp::session`
Enquire_link message can be sent by either the ESME or SMSC and is used to provide a confidence check of the communication path between the two parties, as long as there is an active `async_receive` operation, it would send and receive enquire_link messages and keep the session alive, so there is no need for user intervention.   
The interval for the enquire_link operation can be passed to the constructor of `smpp::session` which has a default value of 60 seconds.
```C++
session::session(asio::ip::tcp::socket socket, std::chrono::seconds enquire_link_interval)
```

#### All the PDUs are aggregates
Being an aggregate made it easy to construct, copy, move and compare PDUs.  
For example, this is what the `cancel_sm` PDU looks like:
```C++
struct cancel_sm
{
  static constexpr auto command_id{ smpp::command_id::cancel_sm };

  std::string service_type{};
  std::string message_id{};
  smpp::ton source_addr_ton{ ton::unknown };
  smpp::npi source_addr_npi{ npi::unknown };
  std::string source_addr{};
  smpp::ton dest_addr_ton{ ton::unknown };
  smpp::npi dest_addr_npi{ npi::unknown };
  std::string dest_addr{};

  bool operator==(const cancel_sm&) const = default;
};
```

We can use C++20's designated initializer with all of the PDUs:
```C++
auto bind_transceiver = smpp::bind_transceiver{
  .system_id = "Example",
  .password = "******",
  .addr_ton = smpp::ton::national,
  .addr_npi = smpp::npi::internet
};
```

#### Optional tag–length–value (TLV) parameters are stored in `smpp::oparam`
If a PDU contains TLV parameters, they will be stored in `oparam` member variable, you can use this member variable to access and manipulate TLV parameters:
```C++
auto deliver_sm = smpp::deliver_sm{ /*...*/ };

deliver_sm.oparam.set_as_string(smpp::oparam_tag::dest_subaddress, "123456789");
deliver_sm.oparam.set_as_enum_u8(smpp::oparam_tag::message_state, smpp::message_state::expired);
```
