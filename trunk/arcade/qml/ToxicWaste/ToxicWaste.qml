import QtQuick 1.1

Rectangle {
    id: toxic_waste_main
    width: 400
    height: 300
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#63b14d"
        }

        GradientStop {
            position: 0
            color: "#3aa82b"
        }

        GradientStop {
            position: 0.740
            color: "#ffffff"
        }

        GradientStop {
            position: 1
            color: "#000000"
        }
    }
}
