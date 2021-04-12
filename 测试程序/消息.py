import socket
import time
import threading
import tkinter as tk

dict_TCP=dict()

class TCP_Thread(threading.Thread):
	def __init__(self,clientsocket):
		threading.Thread.__init__(self)
		#clientsocket.settimeout(10)
		self.clientsocket=clientsocket

	def run(self):
		print("开启新线程")
		tcp_num=0
		while True:
			try:
				bytes=self.clientsocket.recv(1024)#接收套接字
				tcp_num=tcp_num+1
			except:
				break
			try:
				print(str(tcp_num)+":	"+bytes.decode('utf8'))#将受到的内容编码之后输出
			except:
				print(str(tcp_num)+":	"+bytes.decode('gbk'))

			#数据接收完成，处理数据的函数在此插入
	
			#设置要返回的数据
			msg=str(tcp_num)+":	"+'send ok ! 汉字测试'
			try:
				self.clientsocket.send(msg.encode('utf8'))
			except:
				break
		self.clientsocket.close()
		print("一个线程终结")

class UDP_Thread(threading.Thread):
	def run(self):
		print("开启新线程")
		serversocket = socket.socket(socket.AF_INET,socket.SOCK_DGRAM) 
		#启动服务器，""本应填充计算机名或者ip地址，""则为使用本机全部的地址，第二个整型为端口号
		serversocket.bind(("", 9998))
		#设置最大连接数，超过后排队
		print("服务已经启动")
		while True:
			#开始接收数据
			data1,user_add=serversocket.recvfrom(1024)#接收套接字
			try:
				print("UDP "+str(user_add)+":	"+data1.decode('utf8'))#将受到的内容编码之后输出
			except:
				print("UDP "+str(user_add)+":	"+data1.decode('gbk'))
			#print(user_add)
			#serversocket.sendto(data1.upper(),user_add)
			#关闭此连接

#建立UDP接收线程
class TCP_main_Thread(threading.Thread):
	global dict_TCP
	def run(self):
		global dict_TCP
		#主线程里接收TCP #收到之后为每个TCP连接建立独立的接收线程
		#创建 socket 对象
		serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
		#启动服务器，""本应填充计算机名或者ip地址，""则为使用本机全部的地址，第二个整型为端口号
		serversocket.bind(("", 9999))
		#设置最大连接数，超过后排队
		serversocket.listen(5)
		print("服务已经启动")
		tcp_i=0
		while True:
			# 建立客户端连接
			clientsocket,addr = serversocket.accept()#监听链接
			TCP_Thread(clientsocket).start()
			dict_TCP[tcp_i]=clientsocket
			print("连接地址: %s" % str(addr))#输出监听到的链接的地址
			"""
				#开始接收数据
				bytes=clientsocket.recv(1024)#接收套接字
				print(bytes.decode('UTF8'))#将受到的内容编码之后输出

				#数据接收完成，处理数据的函数在此插入
	
				#设置要返回的数据
				msg='汉字测试'+ "\r\n"
				clientsocket.send(msg.encode('UTF8'))
			"""
			#关闭此连接
			#clientsocket.close()


def send_str(str1):
	global dict_TCP
	list_dict = list(dict_TCP.keys())
	for a in dict_TCP:
			try:
				dict_TCP[a].send(str1.encode('utf8'))
				print("##send ok"+str1)
			except:
				dict_TCP.pop(a)
				pass


def tcp_send():
	s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
	try:
		print("尝试发送")
		s.connect(("192.168.128.1", 5045))
		s.settimeout(1)
		s.send("wifi:test123".encode('utf8'))
		msg = s.recv(1024)
		print (msg.decode('utf8'))
		s.send("pawd:86Y1>64m".encode('utf8'))
		msg = s.recv(1024)
		print (msg.decode('utf8'))
		s.close()
		print("发送成功")
	except:
		print("发送失败")


class Application(tk.Frame):
	list1=["@开关1[0-2]:0","@开关1[0-2]:1","@开关1[0-2]:2","@开关2[0-2]:0","@开关2[0-2]:1","@开关2[0-2]:2"]
	def __init__(self, master=None):
		super().__init__(master)
		self.master = master
		self.pack()
		self.create_widgets()

	def create_widgets(self):
		self.hi_there = tk.Button(self)
		#self.hi_there["text"] = "Hello World\n(click me)"
		#self.hi_there["command"] = self.say_hi
		#self.hi_there.pack(side="top")
		self.button1 = tk.Button(self, text="开关1-0", fg="blue",command=self.str_0)
		self.button1.pack()
		self.button1 = tk.Button(self, text="开关1-1", fg="blue",command=self.str_1)
		self.button1.pack()
		self.button1 = tk.Button(self, text="开关1-自动模式", fg="blue",command=self.str_2)
		self.button1.pack()
		self.button1 = tk.Button(self, text="开关2-0", fg="blue",command=self.str_3)
		self.button1.pack()
		self.button1 = tk.Button(self, text="开关2-1", fg="blue",command=self.str_4)
		self.button1.pack()
		self.button1 = tk.Button(self, text="开关2-自动模式", fg="blue",command=self.str_5)
		self.button1.pack()
		self.button1 = tk.Button(self, text="tcp_send", fg="black",command=tcp_send)
		self.button1.pack()
		#self.quit = tk.Button(self, text="QUIT", fg="red",command=self.master.destroy)
		#self.quit.pack(side="bottom")

	def str_0(self):
		send_str(self.list1[0])
	def str_1(self):
		send_str(self.list1[1])
	def str_2(self):
		send_str(self.list1[2])
	def str_3(self):
		send_str(self.list1[3])
	def str_4(self):
		send_str(self.list1[4])
	def str_5(self):
		send_str(self.list1[5])

class app_Thread(threading.Thread):
	def run(self):
		root = tk.Tk()
		app = Application(master=root)
		app.mainloop()

UDP_Thread().start()
TCP_main_Thread().start()
app_Thread().start()

while True:
	str1=input("请输入")
	if str1!="" and str1!="\n":
		print(str1)
		send_str(str1)
	