/**************** NPM PACKAGES ********************/
/* MQTT BROKER PACKAGES */
const aedes = require("aedes")();

/* ENVIRONMENT VARIABLES */
const dotenv = require("dotenv");

/* DATABASE PACKAGES */
const mongoose = require("mongoose");

/***************************************************/

/* DATABASE MODLES */
const Police = require("./models/police");
const Car = require("./models/car");

/* NODE CORE MODULES */
const fs = require("fs");

dotenv.config();

/*********************** MQTT BROKER  ************************/

aedes.authenticate = async function (client, username, password, callback) {
  try {
    await Car.init();
    const car = await Car.findOne({
      carId: username,
      carKey: password.toString(),
    });
    if (!car) {
      let error = new Error("Auth error");
      error.returnCode = 4;
      return callback(error, null);
    }
    if (car.carRole === "police")
      client.subscribe({ topic: "police", qos: 0 }, (error) => {
        if (error) console.log(error);
      });
    client.subscribe({ topic: "arrest", qos: 0 }, (error) => {
      if (error) console.log(error);
    });
    //  car.connected = true;
    // await car.save();
    callback(null, true);
  } catch (error) {
    console.log(error);
  }
};

//const server = require("tls").createServer(options, aedes.handle);
const server = require("net").createServer(aedes.handle);
aedes.authorizePublish = function (client, packet, callback) {
  if (packet.topic === "arrest") {
    if (!client.subscriptions.police)
      return callback(new Error("Not Authorized"));
  }
  callback(null);
};

aedes.on("client", () => {
  console.log("connecting");
});

aedes.on("clientReady", () => {
  console.log("connected");
});

aedes.on("subscribe", (client) => {
  console.log(client);
});

mongoose
  .connect(process.env.MONGO_URI, {
    dbName: process.env.DATABASE_NAME,
  })
  .then(() => {
    server.listen(8883, function () {
      console.log("server started and listening on port ", 8883);
    });
  });
