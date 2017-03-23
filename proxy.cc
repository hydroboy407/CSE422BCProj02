
#include <iostream> 
using std::cout; 
using std::endl;
#include "proxy.h"

/*********************************
 * Name:  ClientHandler
 * Purpose: Handles a single web request
 * Receive: The data socket pointer
 * Return:  None
 *********************************/
void* connectionHandler(void *arg) {
  TCPSocket *clientSock = (TCPSocket *) arg;

  /********TO BE IMPLEMENTED********/

  // Create a ProxyWorker to handle this connection.
  // When done handling the connection, remember to close and delete
  // the socket, and delete the ProxyWorker
  
  ProxyWorker* newProxyWorker = new ProxyWorker(clientSock);
  newProxyWorker->handleRequest();
  clientSock->Close();
  cout << "Connection Served, Proxy child thread terminating" << endl;
  delete clientSock;
  delete newProxyWorker;
  pthread_exit(0);
}

/*********************************
 * Name:    main
 * Purpose: Contains the main server loop that handles requests by 
 *          spawing threads
 * Receive: argc is the number of command line params, argv are the 
 *          parameters
 * Return:  0 on clean exit, and 1 on error
 *********************************/
int main(int argc, char *argv[]) {
  signal(SIGPIPE, SIG_IGN);  // To ignore SIGPIPE
  TCPSocket* clientSock;  // for accepting connections

  int rc;  // return code for pthread

  parseArgs(argc, argv);

  /********TO BE IMPLEMENTED********/
  // Creata a socket, bind it and listen for incoming connection.

  //NEW PART
   //make sure to add try catch statements for these later
   //NOTE FOR ALL IMPLEMENTED CODE: USE TRY CATCH TO SHOW ERRORS

  unsigned short port_num = 0; //is this really correct?
  TCPSocket* proxySock = new TCPSocket();
  proxySock->Bind(port_num);
  proxySock->Listen();
  proxySock->getPort(port_num);

  std::cout << "Proxy running at " << port_num << "..." << std::endl;

  // start the infinite server loop
  while (true) {
    /********TO BE IMPLEMENTED********/
    //break; // remove this break when you have TCPSocket::Accept. This break
           // is to stop the infinite loop from creating too many thread and
           // crashs the program


    // accept incoming connections
   clientSock = proxySock->Accept();
   if(clientSock == NULL) cout << "Did not Accept Client Connection" << endl;

    // create new thread
    pthread_t thread;
    rc = pthread_create(&thread, NULL, connectionHandler, clientSock);

    // if rc != 0, the creation of threadis failed.
    if (rc) {
      std::cerr << "Thread create error! Error code: " << rc << std::endl;
      exit(1);
    }
  }

  /********TO BE IMPLEMENTED********/
  // close the listening sock
  proxySock->Close();

  std::cout << "Parent process termianted." << std::endl;

  return 0;
}
