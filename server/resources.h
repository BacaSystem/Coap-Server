/*
    Prosta reprezentacja grafu za pomoca macierzy sasiedztwa
    Oryginalny kod zrodlowy:
    https://eduinf.waw.pl/inf/alg/001_search/0124.php
*/

class Resources
{
public:
    class Graph
    {
        uint8_t N; //No. nodes
        uint8_t **adjList; //Lista sasiedztwa (tablica 2 wymiarowa)
  
        public:
        
        Graph(const uint8_t n)
        {
            N = n; 
            adjList = new uint8_t* [N];       //Tworzymy tablice wskaznikow
            for (int i = 0; i < N; i++)       //
                adjList[i] = new uint8_t[N];  //Tworzymy wiersze
            
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    adjList[i][j] = 0;        //Wypelniamy zerami

            //Dodajemy przykładowe polaczenia
            AddEdge(0, n-1);
            AddEdge(1, 3);
        }

        bool AddEdge(uint8_t from, uint8_t to)
        {
            if(from == to || from < 0 || to < 0 || from >= N || to >= N)
                return false;            
            else if(adjList[from][to] == 1)
                return false;
            else
            {
                adjList[from][to] = 1;
                return true;
            }
        }

        String GetGraph()
        {
            String s = "";
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++)
                    if(adjList[i][j] == 1)
                        s += String(i)+":"+String(j)+";";
                
            return s;
        }

        ~Graph(){
        for (int i = 0; i < N; i++)
            delete[] adjList[i];
        delete[] adjList;
        }

    };
    
    
    Graph* graph;
    int bytesReceived = 0;
    int bytesSend = 0;
    int bytesTotal = 0;

    String receivedStr = "/ReceivedB";
    String sendStr = "/SendB";
    String totalStr = "/TotalB";
    String graphStr = "/Graph";

    Resources(int n) { graph = new Graph(n); }

    void Received(int bytes) { bytesReceived += bytes; bytesTotal += bytes;}
    void Send(int bytes) { bytesSend += bytes; bytesTotal += bytes;}

    String GetLongResource(String uriPath)
    {
        if(uriPath == totalStr)
            return String(bytesTotal);
        else
            return "";
    }

    String GetResource(String uriPath)
    {
        if(uriPath == receivedStr)
            return String(bytesReceived);
        else if(uriPath == sendStr)
            return String(bytesSend);
        else if(uriPath == totalStr)
            return String(bytesTotal);
        else if(uriPath == graphStr)
            return graph->GetGraph();
        else
            return "";
    }
        
};
