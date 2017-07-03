if (!Detector.webgl) {
            Detector.addGetWebGLMessage();
        }
var container;
var camera, controls, scene, renderer;
var lighting, ambient, keyLight, fillLight, backLight;
var windowHalfX = window.innerWidth / 2;
var windowHalfY = window.innerHeight / 2;
//init();
//animate();
        function init(context) {
            //container = context;//pass in the context that the preview will be put in
            container = document.getElementById("test");
            /* Camera */
            camera = new THREE.PerspectiveCamera(45, $('#'+container.id).width()/ $('#'+container.id).height(), 1, 1000);
            camera.position.z = 40;
            /* Scene */
            scene = new THREE.Scene();
            lighting = false;
            ambient = new THREE.AmbientLight(0xffffff, 1.0);
            scene.add(ambient);
            keyLight = new THREE.DirectionalLight(new THREE.Color('hsl(30, 100%, 75%)'), 1.0);
            keyLight.position.set(-100, 0, 100);
            fillLight = new THREE.DirectionalLight(new THREE.Color('hsl(240, 100%, 75%)'), 0.75);
            fillLight.position.set(100, 0, 100);
            backLight = new THREE.DirectionalLight(0xffffff, 1.0);
            backLight.position.set(100, 0, -100).normalize();
            /* Model */
            // var mtlLoader = new THREE.MTLLoader();
            // mtlLoader.setBaseUrl('assets/');
            // mtlLoader.setPath('assets/');
            // mtlLoader.load('female-croupier-2013-03-26.mtl', function (materials) {
            //     materials.preload();
            //     materials.materials.default.map.magFilter = THREE.NearestFilter;
            //     materials.materials.default.map.minFilter = THREE.LinearFilter;
            //     var objLoader = new THREE.OBJLoader();
            //     objLoader.setMaterials(materials);
            //     objLoader.setPath('assets/');
            //     objLoader.load('female-croupier-2013-03-26.obj', function (object) {
            //         scene.add(object);
            //     });
            // });
            var sphereMaterial = new THREE.MeshLambertMaterial(
            {
              // a gorgeous red.
              color: 0xCC0000
            });
            // create a new mesh with sphere geometry -
            // we will cover the sphereMaterial next!
            var sphere = new THREE.Mesh(
               new THREE.CubeGeometry(20,20,20),

               sphereMaterial);

            // add the sphere to the scene
            scene.add(sphere);
            /* Renderer */
            renderer = new THREE.WebGLRenderer();
            renderer.setPixelRatio(window.devicePixelRatio);
            renderer.setSize(window.innerWidth, window.innerHeight);
            renderer.setClearColor(new THREE.Color("hsl(0, 0%, 10%)"));

            container.appendChild(renderer.domElement);
            //renderer.setSize($('#'+container.id).width(), $('#'+container.id).height());
            console.log("render")
            /* Controls */
            // controls = new THREE.OrbitControls(camera, renderer.domElement);
            // controls.enableDamping = true;
            // controls.dampingFactor = 0.25;
            // controls.enableZoom = false;
            /* Events */
            window.addEventListener('resize', onWindowResize, false);
            window.addEventListener('keydown', onKeyboardEvent, false);
        }
        function initPreview(context){
            document.getElementById(context.id).removeAttribute("onclick");
            init(context);
        }
        function onWindowResize() {
            windowHalfX = window.innerWidth / 2;
            windowHalfY = window.innerHeight / 2;
            camera.aspect = window.innerWidth / window.innerHeight;
            camera.updateProjectionMatrix();
            //renderer.setSize(window.innerWidth, window.innerHeight);
            renderer.setSize(container.width, container.height);
        }
        function onKeyboardEvent(e) {
            if (e.code === 'KeyL') {
                lighting = !lighting;
                if (lighting) {
                    ambient.intensity = 0.25;
                    scene.add(keyLight);
                    scene.add(fillLight);
                    scene.add(backLight);
                } else {
                    ambient.intensity = 1.0;
                    scene.remove(keyLight);
                    scene.remove(fillLight);
                    scene.remove(backLight);
                }
            }
        }
        function animate() {
            requestAnimationFrame(animate);
            controls.update();
            render();
        }
        function render() {
            renderer.render(scene, camera);
        }