import QtQuick 2.6
import QtQuick.Layouts 1.3

Flickable {
	property alias target : introspection.target
	property alias title : introspection.title
	flickableDirection : Flickable.VerticalFlick
    clip: true
	contentWidth : introspection.width
	contentHeight : introspection.height
	implicitWidth : introspection.implicitWidth
	implicitHeight : introspection.implicitHeight
	ObjectIntrospection {
		id: introspection
        width: 300
        height : implicitHeight
	}
}

