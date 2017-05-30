import QtQuick 2.6
import QtQuick.Layouts 1.3

Flickable {
	alias property var target : introspection.target
	alias property var title : introspection.title
	flickableDirection : Flickable.VerticalFlick
	contentWidth : introspection.width
	contentHeight : introspection.height
	implicitWidth : introspection.implicitWidth
	implicitHeight : introspection.implicitHeight
	ObjectIntrospection {
		id: introspection
	}
}

