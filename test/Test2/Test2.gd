extends Node

var port := 45578

var server := SMENetHost.new()
var client := SMENetHost.new()

var server_peer:SMENetPeer
var client_peer:SMENetPeer

func _ready() -> void:
	server.create_server('*', port)
	print('server listen on: %s' % [port])
	client.create_client()
	client.connect_to_host('localhost', port)
	
	server.connect('connected', self, '_on_host_connected', ['Server'])
	client.connect('connected', self, '_on_host_connected', ['Client'])
	server.connect('disconnected', self, '_on_host_disconnected', ['Server'])
	client.connect('disconnected', self, '_on_host_disconnected', ['Client'])
	server.connect('received', self, '_on_host_received', ['Server'])
	client.connect('received', self, '_on_host_received', ['Client'])

func _process(delta: float) -> void:
	server.host_service()
	client.host_service()

func _on_host_connected(peer:SMENetPeer, tag):
	print('[%s] host(%s:%s, %s) connected!' % [tag, peer.get_address(), peer.get_port(), peer.get_connect_id()])
	if tag == 'Server':
		server_peer = peer
	elif tag == 'Client':
		client_peer = peer
	
func _on_host_disconnected(peer:SMENetPeer, tag):
	print('[%s] host(%s:%s, %s) disconnected!' % [tag, peer.get_address(), peer.get_port(), peer.get_connect_id()])
	
func _on_host_received(peer:SMENetPeer, packet:SMENetPacket, channel_id, tag):
	if channel_id == 0:
		print('[%s] from host(%s:%s) on channel[%s]:\n%s' % [tag, peer.get_address(), peer.get_port(), channel_id, packet.get_data_var()])
	elif channel_id == 1:
		print('[%s] channel 1 size: %s' % [tag, packet.get_length()])
		var s := ''
		packet.reset_data_pos(0)
		s += 'byte: %s\n' % packet.get_byte()
		s += 'short: %s\n' % packet.get_short()
		s += 'int: %s\n' % packet.get_int()
		s += 'long: %s\n' % packet.get_long()
		s += 'float: %s\n' % packet.get_float()
		s += 'string: %s\n' % packet.get_string()
		s += 'var: %s\n' % packet.get_var()
		print('[%s] on channel 1:\n %s' % [tag, s])


func _on_ServerButton_pressed() -> void:
	var packet := SMENetPacket.new()
	packet.create()
	packet.put_data_as_var({
		'text': $Control/VBoxContainer/HBoxContainer/ServerLineEdit.text,
	})
	server_peer.send_packet(packet)
	
	


func _on_ClientButton_pressed() -> void:
	var packet := SMENetPacket.new()
	packet.create()
	packet.put_data_as_var($Control/VBoxContainer/HBoxContainer2/ClientLineEdit.text)
	client_peer.send_packet(packet)
	
	packet.create()
	packet.put_byte(123)
	packet.put_short(1234)
	packet.put_int(12345678)
	packet.put_long(87654321)
	packet.put_float(123.456)
	packet.put_string('test测试')
	packet.put_var({'t':123, '测': '试'})
#	server_peer.send_packet(packet, 1)
	client.broadcast(packet, 1)
