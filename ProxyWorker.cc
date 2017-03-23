#include "ProxyWorker.h"
#include <sstream>

const std::string ProxyWorker::subliminalTag = "CSE422";

ProxyWorker::ProxyWorker(TCPSocket *cs) {
  clientSock = cs;
  port = 80;  // For a full blown proxy, the server information should be
              // obtained from each request. However, we simply assume it
              // to be 80 for our labs.
  serverUrl = NULL;  // Must be obtain from each request.
  serverResponse = NULL;
  clientRequest = NULL;
}

ProxyWorker::~ProxyWorker() {
  if (serverUrl) {
    delete serverUrl;
  }

  if (clientRequest) {
    delete clientRequest;
  }

  if (serverResponse) {
    delete serverResponse;
  }
  serverSock.Close();
}

void ProxyWorker::handleRequest() {
  std::string buffer;

  // Get HTTP request from the client, check if the request is valid by
  // parsing it. (parsing is done using HTTPRequest::receive)
  // Please refer to HTTPRequest class for the usage of HTTPRequest::receive
  // and/or HTTPRequest::prarse.
  // From the parsed request, obtain the server address (in code, serverUrl).
  std::cout << "New connection established." << std::endl;
  std::cout << "New proxy child thread started." << std::endl;
  std::cout << "Getting request from client..." << std::endl;

  if (!getRequest()) {
    return;
  }

  // get the request as a std::string
  clientRequest->print(buffer);

  // Just outputting the requrst.
  std::cout << std::endl << "Received request:" << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  clientRequest->print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;

  // Check if the request is valid
  // Terminate this ProxyWorker if it is not a valid request
  std::cout << "Checking request..." << std::endl;
  if (!checkRequest()) {
    return;
  }
  std::cout << "Done. The request is valid." << std::endl;

  std::string host;
  clientRequest->getHost(host);
  std::cout << std::endl << "Forwarding request to server "
            << host << "..." << std::endl;
  if (!forwardRequest()) {
    return;
  }

  // Receive the response header and modify the server header field
  // Receive the response body. Handle the default and chunked transfor
  // encoding.
  std::cout << "Getting the response from the server..." << std::endl;
  if (!getResponse()) {
    return;
  }

  // return the response to the client
  std::cout << "Returning the response to the client..." << std::endl;
  if (!returnResponse()) {
    return;
  }
  return;
}

bool ProxyWorker::getRequest() {
  /********TO BE IMPLEMENTED********/
  // Get the request from the client (HTTPRequest::receive)
  // Chck if the request is received correctly
  //
  // Obtain the serverUrl from the request (HTTPRequest::getUrl)
  
  clientRequest = clientRequest->receive(*clientSock);
  if(clientRequest == NULL) {
     std::cout << "Client Request was not properly received." << std::endl;
  }

  serverUrl = URL::parse(clientRequest->getUrl());
  if(serverUrl == NULL) {
     std::cout << "Server URL could not be parsed." << std::endl;
  }

  return clientRequest == NULL ? false : true; //if clientRequest successfully recorded

}

bool ProxyWorker::checkRequest() {
  // 1. Make sure we're pointing to a server URL
  //    Respond a 404 Not Found if the server is invalid, parse failed
  //    (That is serverUrl == NULL)
  // 2. Filter out any "host" with the keyword "umich.edu"
  //    Note that we are filtering out "host" with "umich.edu".
  //    "path" with umich is allowed.
  //    Respond a 403 forbidden for host with umich.
  // 3. Filter full URL for "harbaugh" or "Harbaugh". If keyword is found,
  //    redirect to host="www.youtube.com" and
  //    path = "/embed/o7iny6VmnlA?autoplay=1"
  //    Note: the request is still valid
  // 4. Insert subliminal message if the requested object is a html and
  //    does not have a subliminal tag

  try { //attempt to connect to the server url
     serverSock.Connect(*serverUrl);
     serverSock.Close(); //close right after, open again in forward request
  }
  catch (std::string msg) { //error thrown if cannot be connected, set serverUrl to null
    serverUrl = NULL;
  }


  if (serverUrl == NULL) {
    /********TO BE IMPLEMENTED********/
    //Respond with 404 Not found
    proxyResponse(404);

    return false;

  } else {  // serverUrl is good

    //filter for umich
    if (serverUrl->getHost().find("umich.edu") != std::string::npos) {
      /********TO BE IMPLEMENTED********/
     //respond with 403 Forbidden
     proxyResponse(403);
     return false;

    //filter url for harbaugh, redirect
    } else if (clientRequest->getUrl().find("harbaugh") != std::string::npos
      || clientRequest->getUrl().find("Harbaugh") != std::string::npos) {
     clientRequest->setHost("cse.msu.edu");
     clientRequest->setPath("/~liuchinj/cse422ss17/lab2_files/whoa.html");
     serverUrl->setHost("cse.msu.edu");
     serverUrl->setPath("/~liuchinj/cse422ss17/lab2_files/whoa.html");

    //insert subliminal message
    } else if (URL::isHtml(clientRequest->getPath()) &&  // 4.
               (!ProxyWorker::hasSubliminalTag(clientRequest->getUrl()))) {
      // Check if this request has subliminalTag
      // If this request does not contain the subliminalTag, the
      // proxy does not forward this request to the server. Instead, the proxy
      // returns a subliminal message response to the client.
      /********TO BE IMPLEMENTED********/
     ProxyWorker::subliminalResponse(clientRequest->getUrl(), 3);

    
    } else if (ProxyWorker::hasSubliminalTag(clientRequest->getUrl())){ // 4.
      // If this request contains the subliminalTag, the request has
      // been served before. The proxy handles the request like a normal proxy.

      // Therefore, we need to remove the subliminalTag
      std::string path = clientRequest->getPath();
      ProxyWorker::removeSubliminalTag(path);
      clientRequest->setPath(path);
    }
  }
  return true;
}

bool ProxyWorker::forwardRequest() {
  // pass the client request to the server
  // connected to the server
  /********TO BE IMPLEMENTED********/

  //connect to the server
  serverSock.Connect(*serverUrl);

  //create get request
  HTTPRequest* request = HTTPRequest::createGetRequest(clientRequest->getUrl());
  
  //get the host, and set it in the request's HOST field
  std::string requested_host;
  clientRequest->getHost(requested_host);
  request->setHost(requested_host);

  //set the connection to non persistent
  request->setHeaderField("Connection", "close");

  //send out the request
  try {
     request->send(serverSock);
  }
  catch (std::string msg) {
     std::cerr << msg << std::endl;
  }
 
  return true;

}

bool ProxyWorker::getResponse() {
  /********TO BE IMPLEMENTED********/
   //The difference between this function and the client.cc is 
   //there is no longer a local file, and instead a string responseContent
   //for holding the entire HTTP response. All occurences of the original fwrite
   //were replaced by responseContent's string concatentation

   //This was copied from the client.cc file
   //Get the header attributes
   std::string responseHeader, responseBody;
   std::string responseContent = "";

  // The client receives the response stream. Check if the data it has
  // contains the whole header.
  // read_header separates the header and data by finding the blank line.
  try {
    serverResponse->receiveHeader(serverSock, responseHeader, responseBody);
  } catch (std::string msg) {
    std::cerr << msg << std::endl;
  }
  std::cout << "Response Header Receieved." << std::endl;
  // The HTTPResponse::parse construct a response object. and check if
  // the response is constructed correctly. Also it tries to determine
  // if the response is chunked transfer encoding or not.
  serverResponse = HTTPResponse::parse(responseHeader.c_str(),
                                 responseHeader.length());

  // The response is illegal.
  if (!serverResponse) {
    std::cerr << "Proxy: Unable to parse the response header." << std::endl;
    // clean up if there's something wrong
    delete serverResponse;
    delete serverUrl;
    exit(1);
  }

  int bytesWritten = 0, bytesLeft;

  std::cout << "Receiving Response Body..." << std::endl;
  if (!(serverResponse->isChunked()) &&  // neither chunked transfer encoding
      serverResponse->getContentLen() == -1) {  // nor default transfer encoding
    std::cout << "The response is neither default tranfer encoding "
              << "nor chunked transfer encoding. This response is not "
              << "supported. Terminating the program without saving the file."
              << std::endl;
  } else if (!(serverResponse->isChunked())) {  
    //Default identity encoding
    // default transfer encoding does not split the data into
    // chunks. The header specifies a Content-Length field. The client knows
    // exactly how many data it is expecting. The client keeps receiving
    // the response until it gets the amount specified.

    std::cout << "Default transfer encoding" << std::endl;
    std::cout << "Content-length: " << serverResponse->getContentLen() << std::endl;
    bytesLeft = serverResponse->getContentLen();

    do {
      responseContent += responseBody;
      bytesWritten += responseBody.length();
      bytesLeft -= responseBody.length();

      std::cout << "bytes written:" <<  bytesWritten << std::endl;
      std::cout << "data gotten:" <<  responseBody.length() << std::endl;

      responseBody.clear();
      try {
        // Keeps receiving until it gets the amount it expects.
        serverResponse->receiveBody(serverSock, responseBody, bytesLeft);
      } catch(std::string msg) {
        // something bad happend
        std::cerr << msg << std::endl;
        // clean up
        delete serverResponse;
        delete serverUrl;
        serverSock.Close();
        exit(1);
      }
    } while (bytesLeft > 0);

  } else {  // chunked encoding
    std::cout << "Chunked transfer encoding" << std::endl;

    // As mentioned above, receiveHeader function already split the
    // body from the header from us. The beginning of this respnse_data
    // now holds the first chunk size.
    int chunkLen = HTTPResponse::getChunkSize(responseBody);
    int totalData = chunkLen;

    while (1) {
      std::cout << "       chunk length: " << chunkLen << std::endl;
      std::cout << "responseBody length: " << responseBody.length()
                << std::endl;
      if (chunkLen == 0) {  // the end of response body
        break;
      } else if (chunkLen == -1) {
        // If chunk length is not found
        // It is possible that the receieveHeader gets exactly only the
        // header and the first chunk length is not recevied yet. In this
        // case, getChunkSize returns -1. Receive more to get the chunk length
        serverResponse->receiveLine(serverSock, responseBody);
        chunkLen = HTTPResponse::getChunkSize(responseBody);
      } else if (responseBody.length() < chunkLen) {
        try {
          // If current data holding is less than the chunkLen, this
          // piece of data contains only part of this chunk. Receive more
          // until we have a complete chunk to store!
          // receive more until we have the whole chunk.
          serverResponse->receiveBody(serverSock, responseBody,
                                (chunkLen - responseBody.length()));
          serverResponse->receiveLine(serverSock, responseBody);
          // get the blank line between chunks
          serverResponse->receiveLine(serverSock, responseBody);
          // get next chunk, at least get the chunk size
        } catch(std::string msg) {
          // something bad happend
          std::cerr << msg << std::endl;
          // clean up
          delete serverResponse;
          delete serverUrl;
          serverSock.Close();
          exit(1);
        }
      } else {  //we have a chunk or more than a chunk of data

   	responseContent += responseBody.substr(0, chunkLen);  //Add chunk to content
        bytesWritten += chunkLen;

        // reorganize the data, remove the chunk from it
        // the + 2 here is to consume the extra CLRF

        //this removes everything prior to the beginning of the new chunk
        responseBody = responseBody.substr(chunkLen + 2,
                           responseBody.length() - chunkLen - 2);
  	
        serverResponse->receiveLine(serverSock, responseBody);
        // get the blank line between chunks
        serverResponse->receiveLine(serverSock, responseBody);
        // get next chunk, at least get the chunk size

        // get next chunk size
        chunkLen = HTTPResponse::getChunkSize(responseBody);

        totalData += chunkLen;
      }
    }

    // This checks if the chunked encoding transfer mode is downloading
    // the contents correctly.
    if ((totalData != bytesWritten) && serverResponse->isChunked()) {
      std::cout << "WARNING" << std::endl
                << "Data received does not match the sum of chunks."
                << std::endl;
    }
    std::cout << "Download complete (" << bytesWritten
              << " bytes written)" << std::endl;
  }


  // If the response is not OK, something is wrong.
  // However, we still downloaded the content, because even the response
  // is not 200. The server still replies with an error page (403, 404 ...)
  if (serverResponse->getStatusCode() != 200) {
    std::cerr << serverResponse->getStatusCode() << " "
              << serverResponse->getStatusDesc() << std::endl;
  }

  // everything's done.
  serverSock.Close();

  serverResponse->setContent(responseContent);  //set the content of the response's body
  std::cout << "Content of Response: " << serverResponse->getContentLen() << std::endl;
  //delete serverUrl;  //old deleted
   
   return true;
}

bool ProxyWorker::returnResponse() {
  /********TO BE IMPLEMENTED********/

  //Set the Server header fields and Transfer Encoding fields to custom settings
  serverResponse->setHeaderField("Server", "wangheilproxy");
  serverResponse->setHeaderField("Transfer-Encoding", "identity");

  //print out response header
  std::string responseHeader;
  serverResponse->print(responseHeader);
  std::cout << "=========================================================="
            << std::endl;
  std::cout << responseHeader << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  
  
  //send server response to client
  try {
    serverResponse->send(*clientSock);
  }
  catch (std::string msg) {
    std::cerr << msg << std::endl;
  }
  //delete serverResponse;  //old deleted

  return true;
}

bool ProxyWorker::hasSubliminalTag(const std::string& url) {
  // Check if the url contains the subliminalTag in its fragment
  URL* requestUrl = URL::parse(url);  // parse it, URL class does it for us
 
  if (!requestUrl) {
    return false;
  }

  int tagPos = url.rfind(subliminalTag);

  // make sure the subliminalTag is at the end of the url.
  if ((tagPos != std::string::npos) &&
      (tagPos + subliminalTag.size() == url.size())) {
    return true;
  } else {
    return false;
  }
}

void ProxyWorker::removeSubliminalTag(std::string& url) {
  size_t tagPos = url.find(subliminalTag);
  // make sure the subliminalTag is at the end of the url.
  if ((tagPos != std::string::npos) &&
      (tagPos + subliminalTag.size() == url.size())) {
    url = url.replace(tagPos, tagPos + subliminalTag.size(), "");
  }
}

bool ProxyWorker::proxyResponse(int statusCode) {
  std::string buffer;
  HTTPResponse proxyResponse(statusCode);

  std::ostringstream oss;
  oss << statusCode;
  std::string page = "<html><body><h1>" + oss.str() + " " +
                     proxyResponse.getStatusDesc() + "</h1></body></html>";
  proxyResponse.setContent(page);

  std::cout << std::endl << "Returning " << statusCode << " to client ..."
            << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  buffer.clear();
  proxyResponse.print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  proxyResponse.send(*clientSock);
  return true;
}

bool ProxyWorker::subliminalResponse(const std::string& url, int duration) {
  std::string buffer;
  // create a new HTTPResponse
  HTTPResponse proxyResponse(200);
  // Randomly choose a image
  int figNumber = rand() % 4;

  std::stringstream ss;
  ss << figNumber;
  std::string figNumberStr = ss.str();
  ss.str("");
  ss << duration;
  std::string durationStr = ss.str();

  // create a webpage containing the image and automatically redirects to
  // original url in "duration" seconds
  proxyResponse.setContent("<html><head><meta http-equiv=\"refresh\" content=\"" + durationStr + ";url=" + url +subliminalTag + "\" /></head><body><center><font size=72>GO GREEN! GO WHITE!</font><br><img src=\"http://www.cse.msu.edu/~liuchinj/cse422ss17/images/" + figNumberStr + ".jpg\" width=800px><br>Redirecting...</center></body></html>");

  std::cout << std::endl << "Returning subliminal to client ..." << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  buffer.clear();
  proxyResponse.print(buffer);
  std::cout << buffer.substr(0, buffer.length() - 4) << std::endl;
  std::cout << "=========================================================="
            << std::endl;
  proxyResponse.send(*clientSock);
  return true;
}
