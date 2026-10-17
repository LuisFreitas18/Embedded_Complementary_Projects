#ifndef OTA_WEBPAGE_H
#define OTA_WEBPAGE_H

const char ota_upload_html[] = R"rawliteral(
<!DOCTYPE html>
<!DOCTYPE html>
<html lang="pt">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>LINCE OTA PANEL</title>
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@400;600;700;800;900&display=swap" rel="stylesheet">
    <link rel="preconnect" href="https://fonts.googleapis.com">
    <link rel="preconnect" href="https://fonts.gstatic.com" crossorigin>
    <link href="https://fonts.googleapis.com/css2?family=Audiowide&family=Orbitron:wght@400..900&family=Zen+Dots&display=swap" rel="stylesheet">
    <style>
        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: 'Inter', -apple-system, sans-serif;
        }

        body, html {
            min-height: 100vh;
            width: 100%;
            background-color: #0b192c;
            color: #ffffff;
            display: flex;
            justify-content: center;
            align-items: flex-start;
            padding: 50px 20px 30px;
        }

        .card_title {
            width: 100%;
            max-width: 600px;
            padding: 0px 16px;
            text-align: center;
        }

        .header-logo {
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 16px;
            margin-bottom: 24px;
        }

        .header-logo img {
            width: 130px; 
            height: 130px;  
            object-fit: contain;
            transform: scale(1.4);
        }

        .title {
            font-size: 4.5rem;
            font-weight: 400;
            letter-spacing: 3px;
            color: #ffffff;
            font-family: 'Audiowide', cursive;
            line-height: 1;
        }

        .card {
            width: 100%;
            max-width: 500px;
            margin: 0 auto;
            background: #112236; /* Fundo do cartão central da foto */
            border-radius: 28px;
            padding: 36px 28px;
            text-align: center;
            box-shadow: 0 10px 30px rgba(0, 0, 0, 0.25);
        }

        .subtitle {
            font-family: 'Orbitron', sans-serif;
            font-size: 2.8rem;
            font-weight: 900;
            font-style: italic;
            text-transform: lowercase;
            letter-spacing: 2px;
            color: #112236;
            margin-bottom: 32px;
            text-shadow: 
                1px 1px 0px #060d17,
                2px 2px 0px #060d17,
                -3px -3px 0px #38dbe3,
                -2px -2px 0px #38dbe3,
                3px 3px 0px #ffffff,
                4px 4px 0px #ffffff;
        }

        /* Botão de escolha do ficheiro (Pill Shape estilo imagem) */
        .drop-zone {
            background: #ffffff;
            border: 2px solid #5a544c;
            border-radius: 50px;
            padding: 14px 28px;
            cursor: pointer;
            margin: 0 auto 24px auto;
            max-width: 280px;
            transition: transform 0.15s ease, border-color 0.15s ease;
            box-shadow: 0 2px 8px rgba(0, 0, 0, 0.15);
        }

        .drop-zone:hover {
            transform: scale(1.02);
            border-color: #c69c3a;
        }

        .drop-zone p {
            font-size: 0.95rem;
            font-weight: 700;
            color: #1a1a1a;
            word-break: break-all;
            transition: color 0.2s ease;
        }

        input[type="file"] {
            display: none;
        }

        /* Botão FLASH Dourado com caixilho/moldura da imagem */
        button {
            width: 120px;
            background: #c69c3a; /* Tom de ouro/ocre da foto */
            color: #ffffff;
            border: 3px solid #6b521d; /* Borda exterior escura */
            outline: 2px solid #ffffff; /* Borda interior em detalhe */
            outline-offset: -5px;
            padding: 10px 0;
            border-radius: 0px; /* Quadrado/Retangular conforme a foto */
            font-weight: 700;
            font-size: 1rem;
            cursor: pointer;
            letter-spacing: 1px;
            margin: 0 auto;
            display: block;
            transition: all 0.15s ease;
        }

        button:hover:not(:disabled) {
            background: #d8aa3f;
            transform: translateY(-1px);
        }

        button:disabled {
            background: #7d6836;
            color: #b0a382;
            border-color: #423617;
            cursor: not-allowed;
            opacity: 0.7;
        }

        .progress-bg {
            background: rgba(255, 255, 255, 0.1);
            border-radius: 10px;
            height: 10px;
            width: 100%;
            overflow: hidden;
            margin-top: 20px;
            display: none;
        }

        .progress-fill {
            background: #ffffff;
            height: 100%;
            width: 0%;
            transition: width 0.1s ease;
        }

        #status {
            font-size: 1.1rem;
            font-weight: 600;
            color: #ffffff;
            margin-top: 28px;
            letter-spacing: 0.5px;
        }

        .error { color: #ff5252 !important; }
        .success { color: #38dbe3 !important; }
    </style>
</head>
<body>

    <div class="card_title">
        <div class="header-logo">
            <img src="https://lh3.googleusercontent.com/d/13z2MAH86dz1SS5QN7OTP56LEyVET1lII" alt="Logo" class="logo-img">
            <div class="title">LINCE</div>
        </div>

        <div class="card">
            <div class="subtitle">ota update</div>

            <form id="uploadForm">
                <div class="drop-zone" onclick="document.getElementById('fileInput').click()">
                    <p id="fileLabel">&lt; firmware.bin &gt;</p>
                </div>
                <input type="file" id="fileInput" name="update" accept=".bin" onchange="onFileSelected()">

                <button type="submit" id="submitBtn" disabled>FLASH</button>
            </form>

            <div class="progress-bg" id="progressBg">
                <div class="progress-fill" id="progressFill"></div>
            </div>
        </div>

        <div id="status">Upload File.</div>
    </div>

<script>
function onFileSelected() {
    const input = document.getElementById('fileInput');
    const label = document.getElementById('fileLabel');
    const btn = document.getElementById('submitBtn');
    const status = document.getElementById('status');

    if (input.files.length > 0) {
        label.innerText = "< " + input.files[0].name + " >";
        label.style.color = "#2dbecf";
        btn.disabled = false;
        status.innerText = "Ready to flash.";
    }
}

document.getElementById('uploadForm').addEventListener('submit', function(e) {
    e.preventDefault();
    const input = document.getElementById('fileInput');
    if (!input.files.length) return;

    const file = input.files[0];
    const xhr = new XMLHttpRequest();
    const btn = document.getElementById('submitBtn');
    const progressBg = document.getElementById('progressBg');
    const progressFill = document.getElementById('progressFill');
    const status = document.getElementById('status');

    btn.disabled = true;
    progressBg.style.display = 'block';
    status.innerText = 'Writing Firmware...';
    status.className = '';

    xhr.upload.onprogress = function(e) {
        if (e.lengthComputable) {
            const percent = Math.round((e.loaded / e.total) * 100);
            progressFill.style.width = percent + '%';
            status.innerText = 'Writing: ' + percent + '%';
        }
    };

    xhr.onload = function() {
        if (xhr.status === 200) {
            status.className = 'success';
            status.innerText = 'Success! Restarting...';
            setTimeout(() => location.reload(), 4000);
        } else {
            status.className = 'error';
            status.innerText = 'ERROR [' + xhr.status + ']';
            btn.disabled = false;
        }
    };

    xhr.onerror = function() {
        status.className = 'error';
        status.innerText = 'Connection Error';
        btn.disabled = false;
    };

    xhr.open('POST', '/update', true);
    xhr.setRequestHeader('Content-Type', 'application/octet-stream');
    xhr.send(file);
});
</script>

</body>
</html>
)rawliteral";

#endif // OTA_WEBPAGE_H