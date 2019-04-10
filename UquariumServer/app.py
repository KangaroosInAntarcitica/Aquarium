from flask import Flask, render_template, request, redirect
import json
import datetime

app = Flask(__name__)

data = {
    "previous_required_temperature": False,
    "required_temperature": 20,
    "current_temperature": False,
    "heater": False,
    "dispenser": False
}


def receive_status(status):
    global data
    status_obj = json.loads(status)

    if "heater" in status_obj:
        data["heater"] = status_obj["heater"]
    if "dispenser" in status_obj and status_obj["dispenser"] == 1:
        data["dispenser"] = False
    if "current_temperature" in status_obj:
        data["current_temperature"] = status_obj["current_temperature"]

    if "required_temperature" in status_obj:
        required_temperature = status_obj["required_temperature"]

        if ("first" in status_obj and status_obj["first"]) or \
            (required_temperature == data["previous_required_temperature"]):
            pass
        else:
            data["required_temperature"] = required_temperature

        data["previous_required_temperature"] = required_temperature

    return \
        str(data["required_temperature"]) + " " + ("1" if data["dispenser"] else "0")


def submit_status():
    global data
    status_obj = dict()
    status_obj["required_temperature"] = data["required_temperature"]
    status_obj["previous_required_temperature"] = data["previous_required_temperature"]
    status_obj["current_temperature"] = data["current_temperature"]
    status_obj["heater"] = data["heater"]
    status_obj["dispenser"] = data["dispenser"]

    return json.dumps(status_obj)


@app.route('/')
def main():
    return redirect('/aquarium')


@app.route('/aquarium', methods=["GET"])
def aquarium():
    return render_template("aquarium.html")


@app.route('/aquarium/data', methods=["GET", "POST"])
def aquarium_data():
    if request.method == "GET":
        return submit_status()
    if request.method == "POST":
        request_data = request.get_data().decode('ascii')
        print("Data received", request_data)
        return receive_status(request_data)


@app.route('/aquarium/data/required_temperature', methods=["POST"])
def receive_required_temperature():
    global data
    request_data = request.get_data().decode('ascii')
    print("Setting required temperature to", request_data)
    data["required_temperature"] = round(float(request_data))
    return "{\"status\": \"OK\"}"


@app.route('/aquarium/data/dispenser_open', methods=["POST"])
def receive_dispenser_open():
    global data
    print("Opening dispenser")
    data["dispenser"] = True
    return "{\"status\": \"OK\"}"


if __name__ == '__main__':
    app.run(debug=True)
