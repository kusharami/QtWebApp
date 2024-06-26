/**
  @file
  @author Stefan Frings
*/

#include "httpconnectionhandler.h"

#include "httpresponse.h"

using namespace qtwebapp;

HttpConnectionHandler::HttpConnectionHandler(const HttpServerConfig &cfg,
	HttpRequestHandler *requestHandler,
	const QSslConfiguration *sslConfiguration)
	: QObject()
	, cfg(cfg)
{
	Q_ASSERT(requestHandler != nullptr);
	this->requestHandler = requestHandler;
	this->sslConfiguration = sslConfiguration;
	currentRequest = nullptr;
	busy = false;
	shouldFinish = false;
	reading = false;

	// execute signals in a new thread
	thread = new QThread();
	thread->start();
	qDebug("HttpConnectionHandler (%p): thread started",
		static_cast<void *>(this));
	moveToThread(thread);
	readTimer.moveToThread(thread);
	readTimer.setSingleShot(true);

	// Create TCP or SSL socket
	createSocket();
	socket->moveToThread(thread);

	// Connect signals
	connect(socket, &QTcpSocket::readyRead, this, &HttpConnectionHandler::read);
	connect(socket, &QTcpSocket::disconnected, this,
		&HttpConnectionHandler::disconnected);
	connect(&readTimer, &QTimer::timeout, this,
		&HttpConnectionHandler::readTimeout);
	connect(thread, &QThread::finished, this, &QObject::deleteLater);

#ifdef CMAKE_DEBUG
	qDebug(
		"HttpConnectionHandler (%p): constructed", static_cast<void *>(this));
#endif
}

HttpConnectionHandler::~HttpConnectionHandler()
{
	readTimer.stop();
	socket->close();
	delete socket;
#ifdef CMAKE_DEBUG
	qDebug("HttpConnectionHandler (%p): destroyed", static_cast<void *>(this));
#endif
}

void HttpConnectionHandler::createSocket()
{
// If SSL is supported and configured, then create an instance of QSslSocket
#ifndef QT_NO_SSL
	if (sslConfiguration)
	{
		QSslSocket *sslSocket = new QSslSocket();
		sslSocket->setSslConfiguration(*sslConfiguration);
		socket = sslSocket;
#ifdef CMAKE_DEBUG
		qDebug("HttpConnectionHandler (%p): SSL is enabled",
			static_cast<void *>(this));
#endif
		return;
	}
#endif
	// else create an instance of QTcpSocket
	socket = new QTcpSocket();
}

void HttpConnectionHandler::handleConnection(qintptr socketDescriptor)
{
	Q_ASSERT(busy);
	Q_ASSERT(!reading);
	Q_ASSERT(
		socket->isOpen() == false); // if not, then the handler is already busy

	// delete previous request
	delete currentRequest;
	currentRequest = nullptr;
#ifdef CMAKE_DEBUG
	qDebug("HttpConnectionHandler (%p): handle new connection",
		static_cast<void *>(this));
#endif
	socket->abort();
	if (!socket->setSocketDescriptor(socketDescriptor))
	{
		busy = false;
		qCritical("HttpConnectionHandler (%p): cannot initialize socket: %s",
			static_cast<void *>(this), qPrintable(socket->errorString()));
		return;
	}

#ifndef QT_NO_SSL
	// Switch on encryption, if SSL is configured
	if (sslConfiguration)
	{
#ifdef CMAKE_DEBUG
		qDebug("HttpConnectionHandler (%p): Starting encryption",
			static_cast<void *>(this));
#endif
		(static_cast<QSslSocket *>(socket))->startServerEncryption();
	}
#endif

	shouldFinish = false;
	// Start timer for read timeout
	readTimer.start(cfg.readTimeout);
}

bool HttpConnectionHandler::isBusy()
{
	return busy;
}

void HttpConnectionHandler::setBusy()
{
	this->busy = true;
}

void HttpConnectionHandler::destroy()
{
	auto thread = this->thread;
	Q_ASSERT(thread);
	thread->requestInterruption();
	thread->quit();
	thread->wait();
	delete thread;
}

void HttpConnectionHandler::readTimeout()
{
	Q_ASSERT(!reading);
	if (!currentRequest)
	{
		readTimer.start(cfg.readTimeout);
		return;
	}
	qDebug("HttpConnectionHandler (%p): read timeout occured",
		static_cast<void *>(this));

	socket->write("HTTP/1.1 408 request timeout\r\nConnection: "
				  "close\r\n\r\n408 request timeout\r\n");

	while (socket->bytesToWrite())
		socket->waitForBytesWritten();
	socket->disconnectFromHost();
	delete currentRequest;
	currentRequest = nullptr;
}

void HttpConnectionHandler::disconnected()
{
#ifdef CMAKE_DEBUG
	qDebug(
		"HttpConnectionHandler (%p): disconnected", static_cast<void *>(this));
#endif
	socket->close();
	if (reading)
	{
		shouldFinish = true;
	} else
	{
		readTimer.stop();
		busy = false;
	}
}

void HttpConnectionHandler::read()
{
	if (reading)
	{
		return;
	}
	reading = true;
	readTimer.stop();
	// The loop adds support for HTTP pipelinig
	while (socket->bytesAvailable())
	{
		if (thread->isInterruptionRequested())
		{
			shouldFinish = true;
			break;
		}
#ifdef SUPERVERBOSE
		qDebug("HttpConnectionHandler (%p): read input",
			static_cast<void *>(this));
#endif

		// Create new HttpRequest object if necessary
		if (!currentRequest)
		{
			currentRequest = new HttpRequest(cfg);
		}

		// Collect data for the request object
		while (socket->bytesAvailable() &&
			currentRequest->getStatus() != HttpRequest::complete &&
			currentRequest->getStatus() != HttpRequest::abort)
		{
			if (thread->isInterruptionRequested())
			{
				shouldFinish = true;
				break;
			}
			currentRequest->readFromSocket(socket);
		}

		// If the request is aborted, return error message and close the connection
		if (currentRequest->getStatus() == HttpRequest::abort)
		{
			if (socket->isOpen())
			{
				socket->write("HTTP/1.1 413 entity too large\r\nConnection: "
							  "close\r\n\r\n413 Entity too large\r\n");
				while (socket->bytesToWrite())
					socket->waitForBytesWritten();
				socket->disconnectFromHost();
			}
			shouldFinish = true;
			delete currentRequest;
			currentRequest = nullptr;
			break;
		}

		// If the request is complete, let the request mapper dispatch it
		if (currentRequest->getStatus() == HttpRequest::complete)
		{
#ifdef CMAKE_DEBUG
			qDebug("HttpConnectionHandler (%p): received request",
				static_cast<void *>(this));
#endif

			// Copy the Connection:close header to the response
			HttpResponse response(socket);
			bool closeConnection =
				QString::compare(currentRequest->getHeader("Connection"),
					"close", Qt::CaseInsensitive) == 0;
			if (closeConnection)
			{
				response.setHeader("Connection", "close");
			}

			// In case of HTTP 1.0 protocol add the Connection:close header.
			// This ensures that the HttpResponse does not activate chunked mode, which is not spported by HTTP 1.0.
			else
			{
				bool http1_0 = QString::compare(currentRequest->getVersion(),
								   "HTTP/1.0", Qt::CaseInsensitive) == 0;
				if (http1_0)
				{
					closeConnection = true;
					response.setHeader("Connection", "close");
				}
			}

			// Call the request mapper
			try
			{
				requestHandler->service(*currentRequest, response);
			} catch (...)
			{
				qCritical("HttpConnectionHandler (%p): An uncatched exception "
						  "occured in the request handler",
					static_cast<void *>(this));
			}

			if (!socket->isOpen() || thread->isInterruptionRequested())
			{
				shouldFinish = true;
			} else
			{
				// Finalize sending the response if not already done
				if (!response.hasSentLastPart())
				{
					response.write(QByteArray(), true);
				}

#ifdef CMAKE_DEBUG
				qDebug("HttpConnectionHandler (%p): finished request",
					static_cast<void *>(this));
#endif

				// Find out whether the connection must be closed
				if (!closeConnection)
				{
					// Maybe the request handler or mapper added a Connection:close header in the meantime
					bool closeResponse =
						QString::compare(
							response.getHeaders().value("Connection"), "close",
							Qt::CaseInsensitive) == 0;
					if (closeResponse)
					{
						closeConnection = true;
					} else
					{
						// If we have no Content-Length header and did not use chunked mode, then we have to close the
						// connection to tell the HTTP client that the end of the response has been reached.
						bool hasContentLength =
							response.getHeaders().contains("Content-Length");
						if (!hasContentLength)
						{
							bool hasChunkedMode =
								QString::compare(response.getHeaders().value(
													 "Transfer-Encoding"),
									"chunked", Qt::CaseInsensitive) == 0;
							if (!hasChunkedMode)
							{
								closeConnection = true;
							}
						}
					}
				}

				// Close the connection or prepare for the next request on the same connection.
				if (closeConnection)
				{
					while (socket->bytesToWrite())
						socket->waitForBytesWritten();
					socket->disconnectFromHost();
					shouldFinish = true;
				}
			}
			delete currentRequest;
			currentRequest = nullptr;
		}
	}
	reading = false;
	if (shouldFinish)
	{
		busy = false;
	} else
	{
		readTimer.start(cfg.readTimeout);
	}
}
