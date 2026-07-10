// ================= FIREBASE =================
const firebaseConfig = {
  apiKey: "AIzaSyBjrGnsdNgZYsW9G3wzxS7jdeIO0BDL52Q",
  authDomain: "monitoringairsumurbor.firebaseapp.com",
  databaseURL: "https://monitoringairsumurbor-default-rtdb.firebaseio.com/",
  projectId: "monitoringairsumurbor",
  storageBucket: "monitoringairsumurbor.firebasestorage.app",
  messagingSenderId: "344311681235",
  appId: "1:344311681235:web:3f13cb6a3fc43d7e3a8426",
  measurementId: "G-L826J3WB9Z"
};

firebase.initializeApp(firebaseConfig);
const database = firebase.database();

// ================= TANGGAL & JAM =================
function updateWaktu() {
    const now = new Date();

    const hari = now.toLocaleDateString("id-ID", {
        weekday: 'long',
        day: '2-digit',
        month: 'long',
        year: 'numeric'
    });

    const jam = now.getHours().toString().padStart(2, '0');
    const menit = now.getMinutes().toString().padStart(2, '0');

    document.getElementById("tanggal").innerHTML = hari;
    document.getElementById("jam").innerHTML = jam + "." + menit;
}

setInterval(updateWaktu, 1000);
updateWaktu();

// ================= AMBIL DATA SENSOR =================
const sensorRef = database.ref("sensor");

sensorRef.on("value", (snapshot) => {
    const data = snapshot.val();

    if (data) {
        // pH - 2 desimal
        document.getElementById("ph").innerHTML = 
            data.ph != null ? parseFloat(data.ph).toFixed(2) : "-";

        // NTU, TDS, Warna - bulat semua
        document.getElementById("ntu").innerHTML = 
            (data.ntu != null ? Math.round(data.ntu) : "-") + " NTU";

        document.getElementById("tds").innerHTML = 
            (data.tds != null ? Math.round(data.tds) : "-") + " ppm";

        document.getElementById("warna").innerHTML = 
            (data.warna != null ? Math.round(data.warna) : "-") + " TCU";

        document.getElementById("kondisi").innerHTML = data.kondisi ?? "-";
        document.getElementById("status").innerHTML  = data.status  ?? "-";

        if (data.kondisi === "LAYAK") {
            document.getElementById("kondisi").style.color = "#4CAF50";
        } else {
            document.getElementById("kondisi").style.color = "#ff5252";
        }
    }
});

function downloadExcel() {

    const dbRef = firebase.database().ref("sensorData");

    dbRef.once("value").then((snapshot) => {

        let dataExcel = [];

        if (snapshot.exists()) {

            snapshot.forEach((childSnapshot) => {
                const data = childSnapshot.val();

                 let waktuFormatted = "-";
    if (data.waktu) {
        const tgl = new Date(data.waktu); // epoch ms dari Firebase
        waktuFormatted = tgl.toLocaleString("id-ID", {
            day: '2-digit', month: '2-digit', year: 'numeric',
            hour: '2-digit', minute: '2-digit', second: '2-digit'
        });
    }
                dataExcel.push({
                    Waktu: waktuFormatted,
                    pH: data.ph || "-",
                    TDS: data.tds || "-",
                    NTU: data.ntu || "-",
                    Warna: data.warna || "-",
                    Kondisi: data.kondisi || "-",
                    Status: data.status || "-"
                });

            });

        } else {

            alert("Belum ada data di database!");

            dataExcel.push({
                Waktu: "-",
                pH: "-",
                TDS: "-",
                NTU: "-",
                Warna: "-",
                Kondisi: "Belum ada data",
                Status: "Belum ada data"
            });

        }

        // membuat worksheet
        const worksheet = XLSX.utils.json_to_sheet(dataExcel);

        // membuat workbook
        const workbook = XLSX.utils.book_new();

        XLSX.utils.book_append_sheet(workbook, worksheet, "Data Sensor");

        // nama file dengan tanggal
        const now = new Date();
        const fileName = "data_kualitas_air_" +
            now.getFullYear() +
            (now.getMonth()+1) +
            now.getDate() + ".xlsx";

        // download file
        XLSX.writeFile(workbook, fileName);

    }).catch((error) => {

        console.error("Error membaca database:", error);
        alert("Gagal mengambil data dari Firebase");

    });

}