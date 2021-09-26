import socket
import json
import sqlite3

# hard-coded users
users = {'Rick': 'glipglop', 'Morty': 'awjeez', 'Summer': 'toocool', 'Jerry': 'weak', 'Beth': 'animals'}

# DB's name
SQL_FILE_NAME = "tweets.db"


# return the HTML file
def returnHTML():
    loginForm = """<!DOCTYPE html>
    <html>
            <head>

            <title>A2Q2 and 3!</title>

            </head>

            <body>

            <form id="login">

            <label for="username">Username?</label> <br>

            <input type="text" id="username" name="username"> <br> <br>

            <label for="password">Password?</label> <br>

            <input type="text" id="password" name="password"> <br> <br>

            <button type="button" onclick="login()">Submit!</button>

            </form>

            <form id="notes">
            <label for="tweet">New Tweet: </label> <br>

            <input type="text" id="tweet" name="tweet"> <br> <br>

            <button type="button" onclick="sendTweet()">Send</button> <br> <br>

            <button type="button" onclick="logout()">Logout</button> <br> <br>

            <div id="display">  <div>
            </form>

            </body>
    </html>"""
    loginLength = str(len(loginForm))
    response = ('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ' + loginLength + '\r\n\r\n'+ loginForm)
    conn.sendall(response.encode())

# login
def login(credentials):
        # credentials is a JSON object which has the form {'username': 'bleh', 'password': blah}
        if 'username' in credentials and 'password' in credentials:
            userVal = credentials['username']
            passVal = credentials['password']
            # Does the provided crecentials match the hard-coded users account
            if credentials['username'] in users and users[userVal] == passVal:
                currentUser = userVal
                login = True
                response = ('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nSet-Cookie: username=' + userVal + '; SameSite=None; HttpOnly; Path=/; Max-Age=99999999\r\nSet-Cookie: password=' + passVal + '; SameSite=None; HttpOnly; Path=/; Max-Age=99999999\r\n\r\n')
                conn.sendall(response.encode())
            else:
                badCredentials()
        else:
            badRequest()

# The provided credentials does not match
def badCredentials():
    message = "Invalid username and password"
    messageLength = str(len(message))
    response = ('HTTP/1.1 401 Unauthorized\r\nContent-Type: text/html\r\nContent-Length: ' + messageLength + '\r\n\r\n'+ message)
    conn.sendall(response.encode())


# depending on the cookie and does it match the hard-coded users, return whether the user is login
def returnLoginStatus(cookie):
        # check if cookie is the 'Cookie' header in the HTTP header
        if 'Cookie' in cookie:
            cookie = cookie.split()
            cookieNum = len(cookie)

            if ('username' in cookie[cookieNum-1]):
                username = cookie[cookieNum-1]
            elif ('username' in cookie[cookieNum-2]):
                username = cookie[cookieNum-2]

            if ('password' in cookie[cookieNum-1]):
                password = cookie[cookieNum-1]
            elif ('password' in cookie[cookieNum-2]):
                password = cookie[cookieNum-2]

            print(username)
            print(password)

            username = username[9:]
            password = password[9:-1]

            if (username in users and users[username] == password):
                message = json.dumps({'authorized': True, 'username': username})
            else: # unlikely to happen since the credentials are from cookie, which means the login is either successful and 'else' will never be reached,
                    # or David Dyck manually set the cookie himself!
                badRequest()
        else:
            message = json.dumps({'authorized': False, 'username': None})

        messageLength = str(len(message))
        response = ('HTTP/1.1 200 OK\r\nContent-Type: application/json\r\nContent-Length: ' + messageLength + '\r\n\r\n'+ message)
        conn.sendall(response.encode())


def getTweet():
    # even if getTweet() is called when there is no post, an appropriate list shall be sent!
    connDB = sqlite3.connect(SQL_FILE_NAME)
    connDB.execute ("""
            Create table if not exists tweets (
                    atTime integer primary key autoincrement,
                    user text,
                    tweet text
            );
    """)

    # Read in the deep thoughts for this user
    tweets = []

    for row in connDB.execute('SELECT * FROM tweets'):
        tweet = "{'user': " + row[1] + ", 'tweet': " + row[2] + ", 'at time': " + str(row[0]) + "}"
        tweets.append(tweet)

    connDB.close()

    print(tweets)
    tweets = str(tweets)
    messageLength = len(tweets)
    messageLength = str(messageLength)

    response = ('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nContent-Length: ' + messageLength + '\r\n\r\n'+ tweets)
    conn.sendall(response.encode())

# post a tweet
def postTweet(tweet, cookie):
    #tweet is a JSON object, and check if cookie is the 'Cookie' header in the HTTP header
    cookie = cookie.split()
    cookieNum = len(cookie)

    if ('username' in cookie[cookieNum-1]):
        username = cookie[cookieNum-1]
    elif ('username' in cookie[cookieNum-2]):
        username = cookie[cookieNum-2]

    username = username[9:]
    print(username)
    print(tweet['tweet'])

    connDB = sqlite3.connect(SQL_FILE_NAME)
    connDB.execute ("""
        Create table if not exists tweets (
                atTime integer primary key autoincrement,
                user text,
                tweet text
        );
    """)

    connDB.execute('insert into tweets(user, tweet) values (?, ?)', (username, tweet['tweet']))
    connDB.commit()

    response = ('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n')
    conn.sendall(response.encode())


def logout():
    currentUser = None
    response = ('HTTP/1.1 200 OK\r\nContent-Type: text/html\r\nSet-Cookie: username=; SameSite=None; Secure; HttpOnly; Path=/; Max-Age=99999999\r\nSet-Cookie: password=; SameSite=None; Secure; HttpOnly; Path=/; Max-Age=99999999\r\n\r\n')
    conn.sendall(response.encode())

def badRequest():
    response = ('HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n')
    conn.sendall(response.encode())



# create an INET, STREAMing socket
serversocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
# bind the socket to a public host, and a well-known port
hostname = socket.gethostname()
print("listening on interface " + hostname)
# This accepts a tuple...
serversocket.bind((socket.gethostname(), 8144))
# become a server socket
serversocket.listen(5)

# multi thread here!
while True:
        conn, addr = serversocket.accept()
        with conn: # this is a socket! With syntax does not work on python 2
                try:
                        print('Connected by', addr)
                        data = conn.recv(1024).decode('UTF-8').split('\r\n\r\n')
                        header = data[0].split("\r\n")
                        request = header[0].split(' ')
                        body = data[1]
                        # cookie tends to be in the last position in the header so assume this is the cookie, will be checked if needs
                        cookie = header[len(header)-1]
                        print(header)
                        print(request)
                        print(body)
                        print(cookie)

                        if request[0] == 'GET':
                            if request[1] == '/':
                                    returnHTML()
                            elif request[1] == '/login':
                                    returnLoginStatus(cookie)
                            elif request[1] == '/feed':
                                    getTweet()
                            else:
                                    response = ('HTTP/1.1 400 Bad Request\r\nContent-Type: text/html\r\n\r\n')
                                    conn.sendall(response.encode())

                        elif request[0] == 'POST':
                                print("right")
                                if len(body) > 0:
                                    try:
                                        credentials = json.loads(body)
                                        print(credentials)
                                        if request[1] == '/login':
                                                login(credentials)
                                        elif request[1] == '/feed' and 'Cookie' in cookie: # is the cookie really the cookie header
                                                print("It's a tweet!")
                                                tweet = credentials
                                                postTweet(tweet, cookie)
                                        else:
                                                badRequest()
                                    except Exception as e:
                                            badRequest()
                                else:                       
                                    if request[1] == '/logout':
                                        logout()
                                    else:
                                        badRequest()
                        else:
                            badRequest()
                except Exception as e:
                    print(e)
        conn.close()


