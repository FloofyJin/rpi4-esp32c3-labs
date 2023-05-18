from http.server import BaseHTTPRequestHandler, HTTPServer
import cgi

class RequestHandler(BaseHTTPRequestHandler):
    
    def do_GET(self):
        self.send_response(200)
        self.send_header('Content-type', 'text/plain')
        self.end_headers()
        self.wfile.write(b'santa cruz')
    
    def do_POST(self):
        ctype, pdict = cgi.parse_header(self.headers.get('content-type'))
        if ctype == 'text/plain':
            length = int(self.headers.get('Content-Length'))
            data = self.rfile.read(length)
            self.send_response(200)
            self.send_header('Content-type', 'text/plain')
            self.end_headers()
            self.wfile.write(b'Received: ' + data)
            print("received data: ", data.decode("utf-8"));
        else:
            self.send_error(400, 'Bad Request: Expected text/plain content type')

def run_server():
    host = '0.0.0.0'
    port = 1234
    server = HTTPServer((host, port), RequestHandler)
    print(f'Starting server at http://{host}:{port}')
    server.serve_forever()

if __name__ == '__main__':
    run_server()

