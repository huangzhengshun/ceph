#include "crimson/common/log.h"
#include "crimson/net/chained_dispatchers.h"
#include "crimson/net/Connection.h"
#include "msg/Message.h"

namespace {
  seastar::logger& logger() {
    return crimson::get_logger(ceph_subsys_ms);
  }
}

seastar::future<>
ChainedDispatchers::ms_dispatch(crimson::net::Connection* conn,
                                MessageRef m) {
  try {
    return seastar::do_for_each(dispatchers, [conn, m](Dispatcher& dispatcher) {
      return dispatcher.ms_dispatch(conn, m);
    }).handle_exception([conn] (std::exception_ptr eptr) {
      logger().error("{} got unexpected exception in ms_dispatch() throttling {}",
                     *conn, eptr);
      ceph_abort();
    });
  } catch (...) {
    logger().error("{} got unexpected exception in ms_dispatch() {}",
                   *conn, std::current_exception());
    ceph_abort();
    return seastar::now();
  }
}

void
ChainedDispatchers::ms_handle_accept(crimson::net::ConnectionRef conn) {
  try {
    for (auto& dispatcher : dispatchers) {
      dispatcher.ms_handle_accept(conn);
    }
  } catch (...) {
    logger().error("{} got unexpected exception in ms_handle_accept() {}",
                   *conn, std::current_exception());
    ceph_abort();
  }
}

void
ChainedDispatchers::ms_handle_connect(crimson::net::ConnectionRef conn) {
  try {
    for(auto& dispatcher : dispatchers) {
      dispatcher.ms_handle_connect(conn);
    }
  } catch (...) {
    logger().error("{} got unexpected exception in ms_handle_connect() {}",
                   *conn, std::current_exception());
    ceph_abort();
  }
}

void
ChainedDispatchers::ms_handle_reset(crimson::net::ConnectionRef conn, bool is_replace) {
  try {
    for (auto& dispatcher : dispatchers) {
      dispatcher.ms_handle_reset(conn, is_replace);
    }
  } catch (...) {
    logger().error("{} got unexpected exception in ms_handle_reset() {}",
                   *conn, std::current_exception());
    ceph_abort();
  }
}

void
ChainedDispatchers::ms_handle_remote_reset(crimson::net::ConnectionRef conn) {
  try {
    for (auto& dispatcher : dispatchers) {
      dispatcher.ms_handle_remote_reset(conn);
    }
  } catch (...) {
    logger().error("{} got unexpected exception in ms_handle_remote_reset() {}",
                   *conn, std::current_exception());
    ceph_abort();
  }
}
