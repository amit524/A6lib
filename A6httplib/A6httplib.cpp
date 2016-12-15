#include <Arduino.h>
#include <A6lib.h>
#include "A6httplib.h"

// String X_IOTFY_ID = "24580020";
// String X_IOTFY_CLIENT = "5dcfb9243fc64d3c1fd9de2a69c5bd";
// String dummy_string="";


A6httplib::A6httplib(A6lib *A6l)
{
    _A6l = A6l;
    
}


A6httplib::~A6httplib()
{

}

bool A6httplib::ConnectGPRS(String apn)
{
	String dummy_string="";
    String _APN=apn;

    byte retstatus=1;

  	while(retstatus != A6_OK)
  	{
	  	retstatus=_A6l->A6command((const char *)"AT+CGATT?", "OK", "yy", 20000, 1, NULL);
		while(_A6l->getFree(3000)!=0);
	}
	retstatus =1;

  	while(retstatus != A6_OK)
  	{
	  	retstatus=_A6l->A6command((const char *)"AT+CGATT=1", "OK", "yy", 20000, 2, NULL);
	  	while(_A6l->getFree(3000)!=0);
	}
	retstatus =1;
	
  	_A6l->A6conn->write("AT+CSQ"); //Signal Quality
  	logln("AT+CSQ");
	delay(200);
	while(_A6l->getFree(3000)!=0);

  	while(retstatus != A6_OK)
  	{
	  dummy_string = "AT+CGDCONT=1,\"IP\",\""+ _APN +"\"";
	  retstatus=_A6l->A6command(dummy_string.c_str(), "OK", "yy", 20000, 2, NULL); //bring up wireless connection
	  while(_A6l->getFree(3000)!=0);
	}
	retstatus =1;
	
	while(retstatus != A6_OK)
	{
	  retstatus=_A6l->A6command((const char *)"AT+CGACT=1,1", "OK", "yy", 10000, 2, NULL);
	  while(_A6l->getFree(3000)!=0);
	 // delay(10000);
	}
}

bool A6httplib::Get(String host, String path)
{
 	char end_c[2];
	end_c[0]=0x1a;
	end_c[1]='\0'; 
    
    _path=path;
    _host=host;

    String body_content="";
    int body_content_start=0;
    int body_content_start_content=0;
    int body_content_end=0;


    dummy_string="GET "+ _path;
    _A6l->A6conn->write(dummy_string.c_str());
    	log("GET "+ _path);
    _A6l->A6conn->write(" HTTP/1.1");
    	log(" HTTP/1.1");
    _A6l->A6conn->write("\r\n");
    	log("\r\n");
    _A6l->A6conn->write("HOST: ");
    	log("HOST: ");
    _A6l->A6conn->write(_host.c_str());
    	log(_host);
    _A6l->A6conn->write("\r\n\r\n");
    	log("\r\n");
	 

	  _A6l->A6command(end_c, "OK", "yy", 30000, 1, NULL); //begin send data to remote server
	  _A6l->A6conn->write(end_c);
	  
	  String rcv="";
	  rcv.reserve(500);
	  String rcv_str="+CIPRCV";

	  char ch='\0';
	  int count_incoming_bytes=0;
	  String body_rcvd_data="";
	  String bytes_in_body="";
	  int bytes_read=0;

	  while(1)
	  	{
	  		if( _A6l->A6conn->available() )
	  			{
	  				rcv="";
	  				rcv=_A6l->A6conn->readStringUntil('\r');
	  				int index=rcv.indexOf('+');
	  				if(index!=-1)
	  				{
	 	 				log(rcv);
	  					logln("found +");
	  					int colon_index=rcv.indexOf(':');
	  					String cutfromrecvd = rcv.substring( index,colon_index);
		  				//logln(cutfromrecvd);
		  				if(cutfromrecvd == rcv_str)
		  					logln("recieving data");
		  				else
		  					logln("not equal");	
	  					
	  					int comma_index=rcv.indexOf(',');
	  					if(comma_index!=-1)
	  					{
		  					bytes_in_body = rcv.substring(colon_index+1, comma_index);
		  					int no_of_bytes=bytes_in_body.toInt();
		  					logln(no_of_bytes);
		  					long now=millis();
		 					long ntime=0;
	  						// int bytes_read=0;
		 					while(1)
	  						{
	  							long ntime=millis();
	  							if(ntime-now>10000 || (bytes_read==no_of_bytes) )
	  							{
	  								log(ntime);
	  								log('\t');
	  								logln(now);
	  								log(bytes_read);
	  								log('\t');
	  								logln(no_of_bytes);
		  							logln(F("timeout occured getting number of bytes"));
		  							break;
	  							}
		 						if(_A6l->A6conn->available())
		  						{
		  							body_rcvd_data += (char) _A6l->A6conn->read();
		  							bytes_read+=1;
		  						}
		  					}
		  					logln(bytes_read);
		  					break;
	   					}

	   					else 
	   						logln("no comma found");
	  				}
	  			}


	  	}
	  	logln(body_rcvd_data);

	  	// String body_content="";
	  	// int body_content_start=body_rcvd_data.indexOf("\n\n");
	  	body_content_start=body_rcvd_data.indexOf("\r\n\r\n");
	  		
	  	log(F("found double newline at  "));
	  	logln(body_content_start);
	  	// int body_content_end=0;
	  	if(body_content_start!=-1)
	  	{
	  		for(int i=body_content_start;i<bytes_read;i++)
	  			if( isAlphaNumeric(body_rcvd_data[i]) )
	  				body_content_start_content=i;

	 // 		body_content_end=body_rcvd_data.indexOf("\r\n",body_content_start+5);
	  			body_content_end = bytes_read;
	  	  	//body_content=body_rcvd_data.substring(body_content_start_content+1,body_content_end);
	  	  	log(F("body content start at "));
	  	  	logln(body_content_start_content+1);
	  	  	log(F("body content end at  "));
	  	  	logln(body_content_end);
	  		// logln(body_content);
	  		// log("check-");
	  		body_content = body_rcvd_data.substring(body_content_start + 4,body_content_end); 
	  		logln(body_content);
	  	}

  logln(F("now going to close tcp connection"));
  _A6l->A6command((const char *)"AT+CIPCLOSE", "OK", "yy", 15000, 1, NULL); 
  delay(100);
  logln("closed");
  _A6l->A6command((const char *)"AT+CIPSTATUS", "OK", "yy", 10000, 2, NULL);
  logln(F("http request done"));


}

