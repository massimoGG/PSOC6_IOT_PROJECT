import socket
import sys
from threading import Thread
import MQTT_decoder
import MQTT_database
from MQTT_packet_handler import packet_router
import time
from MQTT_control_packets import PUBLISH
import MQTT_packet_handler

HOST = "192.168.1.74"
PORT = 1883

connected_clients = []


def main():
    # Initialize database
    MQTT_database.initialize_database()

    # Create dummy session and topic
    #MQTT_database.session_create("test1")
    MQTT_database.topic_create('Temperature')
    MQTT_database.topic_create('Humidity')
    MQTT_database.topic_create('ledstatus')

    # Delete all sessions and topics
    #MQTT_database.topic_delete_all()
    #MQTT_database.session_delete_all()

    start_broker()


def start_broker():

    # Create server socket
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)

    # Bind socket to port
    try:
        server_socket.bind((HOST, PORT))
        print(f"Binding server socket to host: {HOST} and port: {PORT}")
    except:
        print(f"Bind failed. \nError: {str(sys.exc_info())}")
        sys.exit()

    # Enable passive listening sockets
    server_socket.listen(5)

    # Periodically jump out of accept waiting process to receive keyboard interrupt commnad
    server_socket.settimeout(0.5)


    while True:

        client_socket = None

        try:
            # Wait and accept incoming connection
            (client_socket, address) = server_socket.accept()
            ip, port = str(address[0]), str(address[1])
            print(f"Connection from {ip}:{port} has been established.")

            try:
                Thread(target=client_thread, args=(client_socket, ip, port)).start()
                print(f"Client thread for {ip}:{port} has been created.")
            except:
                print(f"Client thread for {ip}:{port} did not create")
        except socket.timeout:
            pass
        except KeyboardInterrupt:
            sys.exit()


def client_thread(client_socket, ip, port):

    global connected_clients

    client_ID = ""

    while True:

        try:
            # Listen to incoming data
            try:
                data = client_socket.recv(1024)
            except:
                print(f'Client ({client_ID}) unexpected disconnect.')
                connected_clients = [client for client in connected_clients if client_ID not in client]
                sys.exit()
            if not data:
                time.sleep(0.5)
                print(f"Client ({client_ID}) went to sleep")
                break

            #print(f"Incomming packet: {data}")

            # Decode incoming packet
            incoming_packet = MQTT_decoder.decode(data)
            #print("The data consists of: "+str(data.encode('UTF-8')))

            if incoming_packet.get("Packet type") == "CONNECT":
                client_ID = incoming_packet.get("Payload")
                connected_clients.append((client_ID, client_socket))

            if incoming_packet.get("Packet type") == "DISCONNECT":
                connected_clients = [client for client in connected_clients if client_ID not in client]

            # Do events & encode outgoing packet
            outgoing_packet = packet_router.route_packet(incoming_packet, client_ID)
            #print(f'Outgoing packet: {outgoing_packet}')


            # Send outgoing packet
            if incoming_packet.get("Packet type") == "PUBLISH":
                send_to_all_connected(outgoing_packet)
            else:
                client_socket.send(outgoing_packet)

            # Send publish packet that has been retained to new subscribers
            if incoming_packet.get("Packet type") == "SUBSCRIBE":
                topic = incoming_packet.get('Topics')[0]
                topic = next(iter(topic))
                if topic in MQTT_database.session_get_topic(client_ID):
                    if MQTT_database.topic_exists(topic):
                        value = MQTT_database.topic_get_value(topic)
                        outgoing_packet = PUBLISH.encode(topic, value)
                        client_socket.send(outgoing_packet)

        except KeyboardInterrupt:
            client_socket.close()
            sys.exit()


def send_to_all_connected(packet: bytes):
    global connected_clients

    for client in connected_clients:
        client_ID, client_socket = client
        client_socket.send(packet)
if __name__ == "__main__":
    main()
