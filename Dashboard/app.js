(function(){
    
    // Inicia o firebase Firebase
    var firebaseConfig = {
      apiKey: "AIzaSyCLNOcFsiju4B0lmO3h1aCl9q-ccMRlr8Q",
      authDomain: "dashboard-9d315.firebaseapp.com",
      databaseURL: "https://dashboard-9d315-default-rtdb.firebaseio.com",
      projectId: "dashboard-9d315",
      storageBucket: "dashboard-9d315.appspot.com",
      messagingSenderId: "72101349374",
      appId: "1:72101349374:web:68c7389e787b8415eaa2af",
      measurementId: "G-1CXQT48E6L"
      };
      // Initialize Firebase
    firebase.initializeApp(firebaseConfig);

  
    var db = firebase.database();
  
    // Cria os listeners dos dados no firebase
    var tempRef = db.ref('DHT11/Temperature');
    var umidRef = db.ref('DHT11/Humidity');
    var umidSoloRef = db.ref('DHT11/UmidadeSolo');
    var motorRef = db.ref('DHT11/Motor');
  
  
    // Registra as funções que atualizam os gráficos e dados atuais da telemetria
    tempRef.on('value', onNewData('currentTemp', 'tempLineChart' , 'Temperatura', 'C°'));
    umidRef.on('value', onNewData('currentUmid', 'umidLineChart' , 'Umidade', '%'));
    umidSoloRef.on('value', onNewData('currentUmidSolo', 'umidSoloLineChart' , 'Umidade Solo', '%'));

    // Registrar função ao alterar valor do motor
    var currentMotorValue = false;
    motorRef.on('value', function(snapshot){
      var value = snapshot.val();
      var el = document.getElementById('currentMotor')
      if(value){
        el.classList.add('blue-text');
        setTimeout(function() {
          currentMotorValue = !!value;
          motorRef.set(!currentMotorValue);
          el.classList.remove('blue-text');
        }, 2000);  
      }
      currentMotorValue = !!value;
    });
  
    // Registrar função de click no botão de motor
    var btnMotor = document.getElementById('btn-motor');
    btnMotor.addEventListener('click', function(evt){
      motorRef.set(!currentMotorValue);
    });
  
  })();
  
  
  // Retorna uma função que de acordo com as mudanças dos dados
  // Atualiza o valor atual do elemento, com a metrica passada (currentValueEl e metric)
  // e monta o gráfico com os dados e descrição do tipo de dados (chartEl, label)
  function onNewData(currentValueEl, chartEl, label, metric){
    return function(snapshot){
      var readings = snapshot.val();
      if(readings){
          var currentValue;
          var data = [];
          for(var key in readings){
            currentValue = readings[key]
            data.push(currentValue);
          }
  
          document.getElementById(currentValueEl).innerText = currentValue + ' ' + metric;
          buildLineChart(chartEl, label, data);
      }
    }
  }
  
  // Constroi um gráfico de linha no elemento (el) com a descrição (label) e os
  // dados passados (data)
  function buildLineChart(el, label, data){
    var elNode = document.getElementById(el);
    new Chart(elNode, {
      type: 'line',
      data: {
          labels: new Array(data.length).fill(""),
          datasets: [{
              label: label,
              data: data,
              borderWidth: 1,
              fill: false,
              spanGaps: false,
              lineTension: 0.1,
              backgroundColor: "#2d3b45",
              borderColor: "#2d3b45"
          }]
      }
    });
  }