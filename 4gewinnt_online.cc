#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>

using namespace std;

struct Grid {
  vector<vector<int>> grid;
  int wd, he;

  Grid(int width, int height) {
    grid.assign(width, vector<int>(height, -1));
    wd = width;
    he = height;
  }

  bool check_sides(int i, int h, int color) {
    int left = i;
    int right = i;
    cout << "Check sides and color " << color << endl;
    cout << "color of board = " << grid[i][h] << endl;
    for (int i = 0; i < 3; ++i) {
      cout << "left: " << left << " right: " << right << endl;
      if (left > 0 && grid[left-1][h] == color) {
        --left;
        cout << left << " left" << endl;
        continue;
      } else if (right+1 < grid.size() && grid[right+1][h] == color) {
        ++right;
        cout << right << " right" << endl;
        continue;
      } else {
        cout << "else" << endl;
        return false;
      }
    }
    if (right - left == 3) {
      return true;
    } else {
      return false;
    }
  }
  bool check_column(int w, int h, int color) {
    int bottom = h;
    int top = h;
    cout << "Check column and color " << color << endl;
    cout << "color of board = " << grid[w][h] << endl;
    for (int i = 0; i < 3; ++i) {
      cout << "top: " << top << " bottom: " << bottom << endl;
      if (bottom > 0 && grid[w][bottom-1] == color) {
        --bottom;
        continue;
      } else {
        return false;
      }
    }
    if (top - bottom == 3) {
      return true;
    } else {
      return false;
    }
  }
  bool check_diagonal(int w, int h, int color) {
    int counter1 = 0;
    int counter2 = 0;
    int bottom_left = 0;
    int top_right = 0;
    cout << "Check diagonal and color " << color << endl;
    cout << "color of board = " << grid[w][h] << endl;
    for (int i = -3; i <= 3; ++i) {
      cout << "counter1: " << counter1 << " counter2: " << counter2 << " bottom_left: " << bottom_left << " top_right: " << top_right << endl;
      if (w+i > 0 && h+i > 0 && w+i < grid.size() && h+i < grid[0].size() && grid[w+i][h+i] == color) {
        ++counter1;
      } else {
        counter1 = 0;
      }
      if (counter1 >= 3) {
        break;
        cout << "win1" << endl;
      }

      if (w-i > 0 && h+i > 0 && w-i < grid.size() && h+i < grid[0].size() && grid[w-i][h+i] == color) {
        ++counter2;
      } else {
        counter2 = 0;
      }
      if (counter2 >= 3) {
        break;
        cout << "win" << endl;
      }
    }
    cout << counter1 << " " << counter2 << endl;
    if (max(counter1, counter2) >= 3) {
      return true; 
    }
    return false;
  }

  bool check_win(int w, int h, int color) {
    return check_diagonal(w, h, color) || check_sides(w, h, color) || check_column(w, h, color);
  }

  int play(int i, int color) {
    int h = -1;
    for (int j = 0; j < grid[0].size(); ++j) {
      if (i >= grid.size() || i < 0) {
        break;
      }
      if (grid[i][j] == -1) {
        grid[i][j] = color;
        h = j;
        break;
      }
    }
    return h;
  }

  void print() {
    for (int i = he-1; i >= 0; --i) {
      for (int j = 0; j < wd; ++j) {
        cout << "\033[44m\033[34m■\033[0m";
        if (grid[j][i] == -1) {
          cout << "| |";
        } else if (grid[j][i] == 0) {
          cout << "|\033[31mO\033[0m|";
        } else {
          cout << "|\033[33mO\033[0m|";
        }
      }
      cout << "\033[44m\033[34m■\033[0m";
      cout << "\n";
    }
    for (int i = 0; i < wd*4.1; ++i) {
      cout << "\033[44m\033[34m■\033[0m";
    }
    cout << "\n";
    cout << " ";
    for (int i = 0; i < wd; ++i) {
      if (i > 0) {
        cout << " ";
      }
      cout << "|" << i+1 << "|";
    }
    cout << "\n\n";
  }
  void paint_line() {
    for (int i = 0; i < wd*4.1; ++i) {
      cout << "-";
    }
    cout << endl;
  }
};

void setup_socket(int &sock, sockaddr_in &addr, bool is_server, int port, string ip_addr) {
  sock = socket(AF_INET, SOCK_STREAM, 0);
  if (sock < 0) {
    perror("Socket creation failed");
    exit(EXIT_FAILURE);
  }

  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = is_server ? INADDR_ANY : inet_addr(ip_addr.c_str());

  if (is_server) {
    int opt = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
      perror("setsockopt failed");
      exit(EXIT_FAILURE);
    }
    if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("Bind failed");
      exit(EXIT_FAILURE);
    }
    if (listen(sock, 1) < 0) {
      perror("Listen failed");
      exit(EXIT_FAILURE);
    }
  } else {
    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
      perror("Connect failed");
      exit(EXIT_FAILURE);
    }
  }
}

int main(int argc, char *argv[]) {
  bool new_round = true;
  int sock, client_sock;
  sockaddr_in addr;
  bool is_server;
  bool gewinner;

  cout << "Are you the server? (1 for Yes, 0 for No): ";
  cin >> is_server;
  gewinner = !is_server;

  int port = 12345; // Default port
  if (argc > 1) {
    port = atoi(argv[1]);
  }
  cout << "Using port: " << port << endl;
  string s = "127.0.0.1";
  if (argc > 2) {
    s = argv[2];
  }
  cout << "Using IP address: " << s << endl;
  setup_socket(sock, addr, is_server, port, s);

  if (is_server) {
    cout << "Waiting for a connection..." << endl;
    socklen_t addr_len = sizeof(addr);
    client_sock = accept(sock, (struct sockaddr *)&addr, &addr_len);
    if (client_sock < 0) {
      perror("Accept failed");
      exit(EXIT_FAILURE);
    }
    cout << "Connection established!" << endl;
  } else {
    client_sock = sock;
    cout << "Connected to the server!" << endl;
  }

  while (new_round) {
    int w, h;
    if (!is_server) {
      cout << "Warte auf den anderen Spieler..." << endl;
      recv(client_sock, &w, sizeof(w), 0);
      recv(client_sock, &h, sizeof(h), 0);
    } else {
      cout << "Wie breit und wie hoch soll das Spielbrett sein?" << endl;
      cout << "Breite? ";
      cin >> w;
      cout << "Hoehe? ";
      cin >> h;
      send(client_sock, &w, sizeof(w), 0);
      send(client_sock, &h, sizeof(h), 0);
    }

    Grid gr(w, h);
    cout << "Du bist Spieler " << (is_server ? "\033[33mGelb\033[0m" : "\033[31mRot\033[0m") << endl;
    int coins = w * h;
    int color = 0;
    bool win = false;
    if (!gewinner) {
      cout << "Welcher Spieler soll anfangen? (0 für Spieler Rot, 1 für Spieler Gelb): ";
      cin >> color;
      send(client_sock, &color, sizeof(color), 0);
    } else {
      cout << "Der andere Spieler wählt aus..." << endl;
      recv(client_sock, &color, sizeof(color), 0);
    }
    cout << "Spieler " << (color ? "\033[33mGelb\033[0m" : "\033[31mRot\033[0m") << " beginnt!" << endl;
    while (coins != 0) {
      int column;
      gr.paint_line();
      gr.print();

      if (color == (is_server ? 1 : 0)) {
        cout << "Welche Spalte wählst du Spieler " << (color ? "\033[33mGelb\033[0m" : "\033[31mRot\033[0m") << "?" << endl;
        cin >> column;

        int height = gr.play(column - 1, color);
        if (column <= 0 || column > w || height == -1) {
          cout << "------------------------------------------------------------" << endl;
          cout << "Vielleicht solltes du eine Spalte nehmen die nicht voll ist." << endl;
          cout << "Du hast aber Glueck, du kannst es nochmals versuchen." << endl;
          continue;
        }

        send(client_sock, &column, sizeof(column), 0);
        if (gr.check_win(column - 1, height, color)) {
          win = true;
        }
        send(client_sock, &win, sizeof(win), 0);
        if (win) {
          gewinner = true;
          break;
        }
      } else {
        cout << "Warten auf den anderen Spieler..." << endl;
        recv(client_sock, &column, sizeof(column), 0);
        gr.play(column - 1, color);
        recv(client_sock, &win, sizeof(win), 0);
        if (win) {
          gewinner = false;
          break;
        }
      }

      --coins;
      color = 1 - color; // Switch color
    }

    cout << "\n\n";
    gr.print();
    if (win) {
      cout << "Spieler " << (color ? "\033[33mGelb\033[0m" : "\033[31mRot\033[0m") << " hat \033[35mgewonnen!\033[0m" << endl;
    } else {
      cout << "\033[32mGleichstand!\033[0m" << endl;
    }

    cout << "Wollt ihr nochmals Spielen? (Ja, Nein)" << endl;
    string s;
    cin >> s;
    if (s != "Ja") {
      new_round = false;
    }
    bool other_new_round;
    if (is_server) {
      cout << "Warten auf den anderen Spieler..." << endl;
      recv(client_sock, &other_new_round, sizeof(new_round), 0);
      if (!new_round || !other_new_round) {
        new_round = false;
      }
      send(client_sock, &new_round, sizeof(new_round), 0);
      if (!new_round) {
        break;
      }
    } else {
      send(client_sock, &new_round, sizeof(new_round), 0);
      cout << "Warten auf den anderen Spieler..." << endl;
      recv(client_sock, &new_round, sizeof(new_round), 0);
      if (!new_round) {
        break;
      }
    }
    cout << "Neue Runde!" << endl;
    cout << "------------------------------------------------------------" << endl;
  }

  close(client_sock);
  close(sock);
  cout << "\033[32mEnde\033[0m" << endl;
}