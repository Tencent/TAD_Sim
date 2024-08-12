// Copyright 2024 Tencent Inc. All rights reserved.
//

#pragma once
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/bind.hpp>
#include <iostream>
#include <istream>
#include <ostream>
#include <string>

using boost::asio::ip::tcp;

namespace https {

class client {
 public:
  client(boost::asio::io_service& io_service, boost::asio::ssl::context& context, const std::string& server,
         const std::string& path)
      : resolver_(io_service), socket_(io_service, context) {
    // Form the request. We specify the "Connection: close" header so that the
    // server will close the socket after transmitting the response. This will
    // allow us to treat all data up until the EOF as the content.
    std::ostream request_stream(&request_);
    request_stream << "GET " << path << " HTTP/1.0\r\n";
    request_stream << "Host: " << server << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "Connection: close\r\n\r\n";

    // Start an asynchronous resolve to translate the server and service names
    // into a list of endpoints.
    tcp::resolver::query query(server, "https");
    resolver_.async_resolve(query, boost::bind(&client::handle_resolve, this, boost::asio::placeholders::error,
                                               boost::asio::placeholders::iterator));
  }

 private:
  void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator) {
    if (!err) {
      std::cout << "Resolve OK"
                << "\n";
      socket_.set_verify_mode(boost::asio::ssl::verify_none);
      socket_.set_verify_callback(boost::bind(&client::verify_certificate, this, _1, _2));

      boost::asio::async_connect(socket_.lowest_layer(), endpoint_iterator,
                                 boost::bind(&client::handle_connect, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Error resolve: " << err.message() << "\n";
    }
  }

  bool verify_certificate(bool preverified, boost::asio::ssl::verify_context& ctx) {
    // The verify callback can be used to check whether the certificate that is
    // being presented is valid for the peer. For example, RFC 2818 describes
    // the steps involved in doing this for HTTPS. Consult the OpenSSL
    // documentation for more details. Note that the callback is called once
    // for each certificate in the certificate chain, starting from the root
    // certificate authority.

    // In this example we will simply print the certificate's subject name.
    char subject_name[256];
    X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
    X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);
    std::cout << "Verifying " << subject_name << "\n";

    return preverified;
  }

  void handle_connect(const boost::system::error_code& err) {
    if (!err) {
      std::cout << "Connect OK "
                << "\n";
      socket_.async_handshake(boost::asio::ssl::stream_base::client,
                              boost::bind(&client::handle_handshake, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Connect failed: " << err.message() << "\n";
    }
  }

  void handle_handshake(const boost::system::error_code& error) {
    if (!error) {
      std::cout << "Handshake OK "
                << "\n";
      std::cout << "Request: "
                << "\n";
      const char* header = boost::asio::buffer_cast<const char*>(request_.data());
      std::cout << header << "\n";

      // The handshake was successful. Send the request.
      boost::asio::async_write(socket_, request_,
                               boost::bind(&client::handle_write_request, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Handshake failed: " << error.message() << "\n";
    }
  }

  void handle_write_request(const boost::system::error_code& err) {
    if (!err) {
      // Read the response status line. The response_ streambuf will
      // automatically grow to accommodate the entire line. The growth may be
      // limited by passing a maximum size to the streambuf constructor.
      boost::asio::async_read_until(
          socket_, response_, "\r\n",
          boost::bind(&client::handle_read_status_line, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Error write req: " << err.message() << "\n";
    }
  }

  void handle_read_status_line(const boost::system::error_code& err) {
    if (!err) {
      // Check that response is OK.
      std::istream response_stream(&response_);
      std::string http_version;
      response_stream >> http_version;
      unsigned int status_code;
      response_stream >> status_code;
      std::string status_message;
      std::getline(response_stream, status_message);
      if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
        std::cout << "Invalid response\n";
        return;
      }
      if (status_code != 200) {
        std::cout << "Response returned with status code ";
        std::cout << status_code << "\n";
        return;
      }
      std::cout << "Status code: " << status_code << "\n";

      // Read the response headers, which are terminated by a blank line.
      boost::asio::async_read_until(socket_, response_, "\r\n\r\n",
                                    boost::bind(&client::handle_read_headers, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Error: " << err.message() << "\n";
    }
  }

  void handle_read_headers(const boost::system::error_code& err) {
    if (!err) {
      // Process the response headers.
      std::istream response_stream(&response_);
      std::string header;
      while (std::getline(response_stream, header) && header != "\r") std::cout << header << "\n";
      std::cout << "\n";

      // Write whatever content we already have to output.
      if (response_.size() > 0) std::cout << &response_;

      // Start reading remaining data until EOF.
      boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1),
                              boost::bind(&client::handle_read_content, this, boost::asio::placeholders::error));
    } else {
      std::cout << "Error: " << err << "\n";
    }
  }

  void handle_read_content(const boost::system::error_code& err) {
    if (!err) {
      // Write all of the data that has been read so far.
      std::cout << &response_;

      // Continue reading remaining data until EOF.
      boost::asio::async_read(socket_, response_, boost::asio::transfer_at_least(1),
                              boost::bind(&client::handle_read_content, this, boost::asio::placeholders::error));
    } else if (err != boost::asio::error::eof) {
      std::cout << "Error: " << err << "\n";
    }
  }

  tcp::resolver resolver_;
  boost::asio::ssl::stream<boost::asio::ip::tcp::socket> socket_;
  boost::asio::streambuf request_;
  boost::asio::streambuf response_;
};

}  // namespace https
