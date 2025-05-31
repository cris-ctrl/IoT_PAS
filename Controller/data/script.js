    let lastA4 = 0, last2C = 0;
    
    function checkStatus() {
      fetch('/status')
        .then(response => response.json())
        .then(data => {
          // Handle A4 updates
          if (data.A4 < lastA4) {
            flash('A4');
          }
          document.getElementById('statusA4').textContent = 
            `${Math.floor(data.A4/1000)} seconds ago`;
          lastA4 = data.A4;

          // Handle 2C updates (using bracket notation for numeric key)
          if (data['2C'] < last2C) {
            flash('2C');
          }
          document.getElementById('status2C').textContent = 
            `${Math.floor(data['2C']/1000)} seconds ago`;
          last2C = data['2C'];

          // Repeat every 100ms
          setTimeout(checkStatus, 100);
        })
        .catch(error => {
          console.error('Status check failed:', error);
          setTimeout(checkStatus, 1000);
        });
    }

    function flash(target) {
      const circle = document.getElementById(`circle${target}`);
      circle.classList.add('green');
      setTimeout(() => {
        circle.classList.remove('green');
      }, 200);
    }

    function toggle(target, state) {
      fetch(`/toggle?target=${target}&state=${state ? 1 : 0}`)
        .catch(error => console.error('Toggle failed:', error));
    }

    // Start status checks when page loads
    checkStatus();