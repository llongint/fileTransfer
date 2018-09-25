## 我的编译环境
- Linux ubuntu 4.15.0-33-generic #36~16.04.1-Ubuntu SMP Wed Aug 15 17:21:05 UTC 2018 x86_64 x86_64 x86_64 GNU/Linux
- 如果不一样，重新编译就行
   - make clean
   - make

---
## 测试步骤(```必须使用超级权限```,ubuntu用sudo,centos用yum)
- sudo ./server 10086 &
   - 端口号随意,但是首次运行的时候需要等一下，因为创建秘钥需要时间

- sudo ./client 127.0.0.1 10086
   - 同上，首次运行的时候需要等一下，因为创建秘钥需要时间

- regist
   - 注册新用户，根据提示输入姓名，密码，```不要填中文```，验证码时死的，即identifying code: 076923
- login
   - 根据刚刚注册的填写，登录成功会答应收到的session(你可以自己注释掉不打印)
- ls
   - 查看服务端的文件，一开始肯定只有 . ..两个目录
- ```ctrl + T```
  - ```重新打开一个shell窗口，拷贝一个ASCII文件进去，方便测试```
- sudo cp client.c /home/fileTransfer.client/
   - 拷贝完后回到刚才那个用户窗口
- put client.c
   - 上传文件client.c到服务端
   - ls  /home/fileTransfer.server/80181560622/ 进行查看
   - cat /home/fileTransfer.server/80181560622/client.c 打开可以看到是一个二进制文件
   - ```注意80181560622这几个数字是用户标识码，注册的时候随机生成的，登录的时候会进入到这个目录，就是说不同的用户进入的目录是不同的```
- get client.c
   - 把服务端的的文件下载到本地，本地有相同名字的文件会进程重命名一个随机的名字
- cat /home/fileTransfer.client/0765
   - 通过上面的命令看到文件已经解密完成

---
## 代码整体的逻辑

- 1.服务端主要逻辑
   - 1.初始化工作目录，将工作目录切换到"/home/fileTransfer.server",并在该目录下创建秘钥文件(```当前的版本没有用到```)和用户配置信息文件```"serv.user.data"```
   - 2.创建TCP服务，用select()监听
   - 3.因为每个客户需要一个独立的工作目录，所以为每一个连接创建一个新的子进程
      - 3.1 client_handle(),每个子进程服监听一个连接套接字
      - 3.2 在client_handle()中循环等待客户端的数据
      - 3.3 在收到数据后先读文件头，每个消息都用的TLV格式，消息头包含类型和长度两个信息，T代表消息的类型，L代表消息的长度，V就是具体消息的内容，比如消息头是0x0001 0000 000A,就表示后面消息的类型是0001(登录信息),消息的长度是10个字节，```注意：这里的大小端问题因为找不到两台本地字节序不同的主机，没办法测试，就没考虑了```
      - 3.4 在解析出具体的消息类型后，再调用delServerRecv()进行处理

- 2.客户端主要逻辑
   - 1.从标准输入接收ip和端口号
   - 2.file_init()切换工作目录并创建秘钥文件，用来在文件传输前进行加密，并在接收文件后进行解密，```注意：请不要尝试二进制文件，只是课程设计，我没考虑那么多```
   - 3.与服务器连接后循环监听连接套接字与标准输入
   - 4.检测到标准输入用delStdinInput()进行处理，检测到服务端数据用readClientRecv()进行处理

- 3.```登录与注册```
   - 1.客户端再标准输入检测到输入"regist",调用sendRegisterCmd(),向服务端发送一个e_msgRegist消息
   - 2.服务端接收到e_msgRegist类型消息后save_userData()保存用户名和密码到serv.user.data"文件和链表
   - 3.客户端再标准输入检测到输入"login",调用sendLoginCmd(),发送一个e_msgLogin的消息
   - 4.服务端接收到e_msgLogin类型的消息后调用user_confirmation(),在链表中查找是否存在匹配的用户名和密码，查找到则发送session给客户端，并切换以用户账号作为目录名的目录(用户账号在用户注册的时候随机生成)，客户端以后每次请求都要带上32位二进制的session