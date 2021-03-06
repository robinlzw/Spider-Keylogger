#include "Network.hpp"

#include "AFactory.hpp"

#include "BoostOpenSslListenSocket.hpp"
#include "BoostListenSocket.hpp"
#include "BoostSslCtxServer.hpp"

Network::Network(const std::string &addr, uint16_t port, ALog& log)
  : _ssl(new BoostSslCtxServer()),
    _acceptor(new BoostOpenSslListenSocket(_ssl)),
   _log(log)
{
  _acceptor->bind(addr, port);
  _acceptor->listen(20);
  queueAccept();
}

void	Network::poll_clients()
{
  _acceptor->poll();
}

void    Network::run()
{
  _acceptor->start();
}

void    Network::stop()
{
  _acceptor->stop();
}

void    Network::registerSpider(const std::shared_ptr<Spider>& spider)
{
  std::cout << "Spider registered with id: " << spider->getIdentity() << std::endl;
  _clients.insert(spider);
}

void      Network::unregisterSpider(const std::shared_ptr<Spider>& spider)
{
  _clients.erase(spider);
}

void	Network::broadcast(const std::vector<uint8_t> &buffer)
{
  for (auto &client : _clients) {
    std::function<void(size_t)> w(std::bind(&Network::onWrite, this, client, std::placeholders::_1));
    client->getSocket()->async_write(buffer, w);
  }
}

void	Network::queueAccept()
{
  auto acc(std::bind(&Network::onAccept, this, std::placeholders::_1));

  _acceptor->async_accept(acc);
}

void	Network::onAccept(const std::shared_ptr<IConnectSocket>& newSock)
{
  std::shared_ptr<Spider> spider(new Spider(newSock, *this, _log));
  std::cout << "New client." << std::endl;
  spider->spy();
}

void	Network::onWrite(const std::shared_ptr<Spider>& spider, std::size_t size)
{
  (void)spider;
  std::cout << "Sent a message to a spider (" << size << ")" << std::endl;
}
