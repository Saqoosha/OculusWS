/**
 * @author Saqoosha / http://saqoo.sh/a
 *
 * Based on possan's OculusControl.js
 */

THREE.OculusControls = function (object) {
    this.object = object;
    this.target = new THREE.Vector3(0, 0, 0);
    this.headquat = new THREE.Quaternion();
    this.freeze = false;
    var socket = null;

    this.update = function (delta) {
        if (this.freeze) {
            return;
        }
        this.object.quaternion.multiply(this.headquat);
    };

    var scope = this;
    this.connect = function () {
        socket = new WebSocket('ws://localhost:7681');
        socket.onopen = function (event) {
            console.log('open!', arguments);
        };
        socket.onmessage = function (event) {
            try {
                var data = JSON.parse(event.data);
                if (data.address == '/rift/orientation') {
                    scope.headquat.set(data.args[0], data.args[1], data.args[2], data.args[3]);
                }
            } catch (e) {
                console.log('message parse error', e, arguments);
            }
        };
        socket.onclose = function (event) {
            console.log('close!', arguments);
        };
        socket.onerror = function (event) {
            console.log('error', arguments);
        };
    };
};
