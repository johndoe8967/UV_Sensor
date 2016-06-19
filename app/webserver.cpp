/*
 * webserver.cpp
 *
 *  Created on: 19.06.2016
 *      Author: johndoe
 */

#include "webserver.h"

int totalActiveSockets = 0;
HttpServer server;

WebSocketsList &clients=server.getActiveWebSockets();

void onIndex(HttpRequest &request, HttpResponse &response)
{
	TemplateFileStream *tmpl = new TemplateFileStream("index.html");
	auto &vars = tmpl->variables();
	//vars["counter"] = String(counter);
	response.sendTemplate(tmpl); // this template object will be deleted automatically
}

void onFile(HttpRequest &request, HttpResponse &response)
{
	String file = request.getPath();
	if (file[0] == '/')
		file = file.substring(1);

	if (file[0] == '.')
		response.forbidden();
	else
	{
		response.setCache(86400, true); // It's important to use cache for better performance.
		response.sendFile(file);
	}
}

void wsConnected(WebSocket& socket)
{
	totalActiveSockets++;

	// Notify everybody about new connection
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("{\"type\": \"JSON\", \"msg\": \"New friend arrived!\", \"value\": " + String(totalActiveSockets) + "}");
}

void wsMessageReceived(WebSocket& socket, const String& message)
{
	Serial.printf("WebSocket message received:\r\n%s\r\n", message.c_str());
	String response = "Echo: " + message;
	socket.sendString(response);
}

void wsBinaryReceived(WebSocket& socket, uint8_t* data, size_t size)
{
	Serial.printf("Websocket binary data recieved, size: %d\r\n", size);
}

void wsDisconnected(WebSocket& socket)
{
	totalActiveSockets--;

	// Notify everybody about lost connection
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("{\"type\": \"JSON\", \"msg\": \"We lost our friend :(\", \"value\": " + String(totalActiveSockets) + "}");
}

void sendMeasureToClients(uint value, float avgValue) {
	for (int i = 0; i < clients.count(); i++)
		clients[i].sendString("{\"type\": \"JSON\", \"msg\": \"UVValue\", \"value\": " + String(value) +", \"avgValue\": " + String(avgValue) + "}");
}

void startWebServer()
{
	server.listen(80);
	server.addPath("/", onIndex);
	server.setDefaultHandler(onFile);

	// Web Sockets configuration
	server.enableWebSockets(true);
	server.setWebSocketConnectionHandler(wsConnected);
	server.setWebSocketMessageHandler(wsMessageReceived);
	server.setWebSocketBinaryHandler(wsBinaryReceived);
	server.setWebSocketDisconnectionHandler(wsDisconnected);

	Serial.println("\r\n=== WEB SERVER STARTED ===");
	Serial.println(WifiStation.getIP());
	Serial.println("==============================\r\n");

}
