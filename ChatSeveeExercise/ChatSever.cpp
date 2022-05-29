#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib, "ws2_32")
#pragma warning(disable:4996)

SOCKET clients[64];
int numClients = 0;
void RemoveClient(SOCKET client)
{
    int i = 0;
    for (; i < numClients; i++)
        if (clients[i] == client) break;
    // Xoa client khoi mang
    if (i < numClients - 1)
        clients[i] = clients[numClients - 1];
    numClients--;
}

DWORD WINAPI ClientThread(LPVOID lpParam)
{
    SOCKET client = *(SOCKET*)lpParam;
    int ret;
    char buf[256];
    char user[32], pass[32], tmp[32];
    // Xu ly dang nhap
    while (1)
    {
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
            return 0;
        buf[ret] = 0;
        printf("Du lieu nhan duoc: %s\n", buf);
        ret = sscanf(buf, "%s %s %s", user, pass, tmp);
        if (ret != 2)
        {
            const char* msg = "Sai cu phap dang nhap. Hay nhap lai !!!\n";
            send(client, msg, strlen(msg), 0);
        }
        else
        {
            FILE* f = fopen("D:\\danhsach.txt", "r");
            int found = 0;
            while (!feof(f))
            {
                char line[256];
                fgets(line, sizeof(line), f);
                char userf[32], passf[32];
                sscanf(line, "%s %s", userf, passf);
                if (strcmp(user, userf) == 0 && strcmp(pass, passf) == 0)
                {
                    found = 1;
                    break;
                }
            }
            fclose(f);
            if (found == 1)
            {
                const char* msg = "Dang nhap thanh cong. Hay nhap lenh !!!\n";
                send(client, msg, strlen(msg), 0);
                //Thong co khi co Client moi duoc ket noi
                const char* newMsg = "Client moi ket noi\n";
                char newBuf[256];
                sprintf(newBuf, "%s: %s", user, newMsg);
                for (int i = 0; i < numClients; i++) {
                    send(clients[i], newBuf, strlen(newBuf), 0);
                }
                //Them client vao mang
                clients[numClients] = client;
                numClients++;
                break;
            }
            else
            {
                const char* msg = "Khong tim thay tai khoan. Hay nhap lai !!!\n";
                send(client, msg, strlen(msg), 0);
            }
        }
    }
    // Gui tin nhan
    while (1)
    {
        char cmd[32];
        ret = recv(client, buf, sizeof(buf), 0);
        if (ret <= 0)
        {
            RemoveClient(client);
            return 0;
        }
        buf[ret] = 0;
        printf("Du lieu nhan duoc: %s\n", buf);
        // Xu ly du lieu
        // Doc tu dau tien va luu vao cmd
        sscanf(buf, "%s", cmd);
        char sbuf[256];
        int id;
        sprintf(sbuf, "%s: %s", user, buf + 3);
        if (strcmp(cmd, "all") == 0)    //chat voi tat ca cac client
        {
            // Gui cho cac client khac
            for (int i = 0; i < numClients; i++)
            {
                char* msg = buf + strlen(cmd) + 1; // message gui di bat dau tu ki tu tiep theo sau cmd
                send(clients[i], sbuf, strlen(sbuf), 0);
            }
        }
        //Dang xuat
        else if (strcmp(cmd, "exit") == 0) {
            const char* msg = "Ban da dang xuat.\n";
            send(client, msg, strlen(msg), 0);
            //Gui thong bao co client dang xuat
            const char* newMsg = "Client dang xuat.\n";
            char newBuf[256];
            sprintf(newBuf, "%s: %s", user, newMsg);
            for (int i = 0; i < numClients; i++) {
                if (clients[i] != client) {
                    send(clients[i], newBuf, strlen(newBuf), 0);
                }
            }
            RemoveClient(client);
            return 0;
        }
        //Lay danh sach cac client dang nhap
          
        else  // Chat ca nhan
        {
            id = atoi(cmd); // chuyen chuoi cmd sang so
            // Gui cho client id
            for (int i = 0; i < numClients; i++)
                if (clients[i] == id)
                {
                    char* msg = buf + strlen(cmd) + 1;
                    send(clients[i], sbuf, strlen(sbuf), 0);
                }
        }
    }
    closesocket(client);
    WSACleanup();
}
int main()
{
    // Khoi tao thu vien
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    // Tao socket
    SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    // Khai bao dia chi server
    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);
    // Gan cau truc dia chi voi socket
    bind(listener, (SOCKADDR*)&addr, sizeof(addr));
    // Chuyen sang trang thai cho ket noi
    listen(listener, 5);
    while (1)
    {
        SOCKET client = accept(listener, NULL, NULL);
        printf("Client moi ket noi: %d\n", client);
        CreateThread(0, 0, ClientThread, &client, 0, 0);
    }
}
