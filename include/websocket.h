/*******************************************************************************
 * ARICPP - ARI interface for C++
 * Copyright (C) 2017-2021 Daniele Pallastrelli
 *
 * This file is part of aricpp.
 * For more information, see http://github.com/daniele77/aricpp
 *
 * Boost Software License - Version 1.0 - August 17th, 2003
 *
 * Permission is hereby granted, free of charge, to any person or organization
 * obtaining a copy of the software and accompanying documentation covered by
 * this license (the "Software") to use, reproduce, display, distribute,
 * execute, and transmit the Software, and to prepare derivative works of the
 * Software, and to permit third-parties to whom the Software is furnished to
 * do so, all subject to the following:
 *
 * The copyright notices in the Software and this entire statement, including
 * the above license grant, this restriction and the following disclaimer,
 * must be included in all copies of the Software, in whole or in part, and
 * all derivative works of the Software, unless such copies or derivative
 * works are solely in the form of machine-executable object code generated by
 * a source language processor.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, TITLE AND NON-INFRINGEMENT. IN NO EVENT
 * SHALL THE COPYRIGHT HOLDERS OR ANYONE DISTRIBUTING THE SOFTWARE BE LIABLE
 * FOR ANY DAMAGES OR OTHER LIABILITY, WHETHER IN CONTRACT, TORT OR OTHERWISE,
 * ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 ******************************************************************************/

#ifndef ARICPP_WEBSOCKET_H_
#define ARICPP_WEBSOCKET_H_

//#define ARICPP_TRACE_WEBSOCKET

#include <string>
#include <utility>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#ifdef ARICPP_TRACE_WEBSOCKET
#include <iostream>
#endif
#include "./aricpp/detail/boostasiolib.h"

namespace aricpp
{

class WebSocket
{
public:
    using ConnectHandler = std::function<void(const boost::system::error_code&)>;
    using ReceiveHandler = std::function<void(const std::string&, const boost::system::error_code&)>;

   WebSocket(detail::BoostAsioLib::ContextType& _ios, std::string _host, std::string _port) :
        ios(_ios),
        host(std::move(_host)),
        port(std::move(_port)),
        resolver(ios),
        socket(ios),
        websocket(socket),
        pingTimer(ios)
    {}

    WebSocket() = delete;
    WebSocket(const WebSocket&) = delete;
    WebSocket(WebSocket&&) = delete;
    WebSocket& operator=(const WebSocket&) = delete;
    WebSocket& operator=(WebSocket&&) = delete;

    ~WebSocket() noexcept { Close(); }

    void Connect(std::string req, ConnectHandler h, const std::chrono::seconds& _connectionRetry)
    {
        request = std::move(req);
        onConnection = std::move(h);
        Resolve();
        connectionRetry = _connectionRetry;
        if (connectionRetry != std::chrono::seconds::zero())
            StartPingTimer();
    }

    void Close() noexcept
    {
        try
        {
            if (socket.is_open())
            {
                if (connected)
                    websocket.close(boost::beast::websocket::close_code::normal);
                socket.shutdown(boost::asio::ip::tcp::socket::shutdown_send);
                socket.cancel();
                socket.close();
            }
        }
        catch (const std::exception&)
        {
            // nothing to do
        }
        connected = false;
    }

    void Receive(ReceiveHandler h)
    {
        onReceive = std::move(h);
        Read();
    }

private:
    void Ping()
    {
        if (connected)
        {
            boost::beast::websocket::ping_data pingWebSocketFrame;
            pingWebSocketFrame.append("aricpp");
            websocket.async_ping(
                pingWebSocketFrame,
                [this](const boost::beast::error_code& error_code)
                {
                    if (error_code)
                    {
                        // remedy the situation. Close the current websocket connection and try to restart
                        Close();
                        Resolve();
                    }
                });
        }
        else
        {
            Resolve();
        }
    }

    void Resolve()
    {
        resolver.async_resolve(
            host,
            port,
            [this](const boost::system::error_code& ec, boost::asio::ip::tcp::resolver::results_type i)
            {
                if (ec)
                    onConnection(ec);
                else
                    Resolved(i);
            });
    }

    void PingTimerExpired()
    {
        Ping();
        StartPingTimer();
    }

    void StartPingTimer()
    {
        if (connectionRetry == std::chrono::seconds::zero()) return;

        pingTimer.expires_after(connectionRetry);
        pingTimer.async_wait(
            [this](const boost::system::error_code& e)
            {
                if (e) return;
                PingTimerExpired();
            });
    }

    void Resolved(boost::asio::ip::tcp::resolver::results_type i)
    {
        boost::asio::async_connect(
            socket,
            std::move(i),
            [this](const boost::system::error_code& e, const boost::asio::ip::tcp::endpoint&)
            {
                if (e)
                    onConnection(e);
                else
                    Connected();
            });
    }

    void Connected()
    {
        websocket.async_handshake(
            host,
            request,
            [this](boost::beast::error_code ec)
            {
                connected = true;
                onConnection(ec);
            });
    }

    void Read()
    {
        websocket.async_read(
            rxData,
            [this](boost::system::error_code ec, std::size_t /* bytes */) { Received(ec); }
        );
    }

    void Received(boost::system::error_code ec)
    {
        std::string s((std::istreambuf_iterator<char>(&rxData)), std::istreambuf_iterator<char>());
#ifdef ARICPP_TRACE_WEBSOCKET
        if (ec)
            std::cerr << "*** websocket error: " << ec.message() << '\n';
        else
            std::cerr << "*** <== " << s << '\n';
#endif
        if (ec)
            onReceive(std::string(), ec);
        else
            onReceive(s, ec);
        rxData.consume(rxData.size());
        if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted) Read();
    }

    bool connected = false;
    detail::BoostAsioLib::ContextType& ios;
    const std::string host;
    const std::string port;

    boost::asio::ip::tcp::resolver resolver;
    boost::asio::ip::tcp::socket socket;
    boost::beast::websocket::stream<boost::asio::ip::tcp::socket&> websocket;
    boost::asio::streambuf rxData;

    std::string request;
    ConnectHandler onConnection;
    ReceiveHandler onReceive;

    boost::asio::steady_timer pingTimer;
    std::chrono::seconds connectionRetry = std::chrono::seconds::zero();
};

} // namespace aricpp

#endif
