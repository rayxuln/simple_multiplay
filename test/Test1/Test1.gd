extends Node



func _on_ServerButton_pressed() -> void:
	$SMTestNode.server_send_msg($Control/VBoxContainer/HBoxContainer/ServerLineEdit.text)


func _on_ClientButton_pressed() -> void:
	$SMTestNode.client_send_msg($Control/VBoxContainer/HBoxContainer2/ClientLineEdit.text)
