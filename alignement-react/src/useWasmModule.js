import { useState, useEffect } from 'react';

export function useWasmModule(moduleUrl) {
  const [module, setModule] = useState(null);
  const [error, setError] = useState(null);
  const [isLoading, setIsLoading] = useState(true);

  useEffect(() => {
    let isMounted = true;

    const loadWasmModule = async () => {
      try {
        // Charger le script Emscripten généré
        const script = document.createElement('script');
        script.src = moduleUrl.replace('.wasm', '.js');
        script.async = true;
        document.body.appendChild(script);

        // Attendre que le script soit chargé
        await new Promise((resolve, reject) => {
          script.onload = resolve;
          script.onerror = reject;
        });

        // Le script Emscripten définit une fonction globale Module
        // Attendons qu'elle soit disponible
        const waitForModule = () => {
          return new Promise((resolve) => {
            const check = () => {
              if (window.Module) {
                resolve(window.Module);
              } else {
                setTimeout(check, 10);
              }
            };
            check();
          });
        };

        const Module = await waitForModule();
        
        // Initialiser le module
        const initializedModule = await Module();

        if (isMounted) {
          setModule(initializedModule);
          setIsLoading(false);
        }
      } catch (err) {
        console.error("Erreur lors du chargement du module WebAssembly:", err);
        if (isMounted) {
          setError(err);
          setIsLoading(false);
        }
      }
    };

    loadWasmModule();

    return () => {
      isMounted = false;
    };
  }, [moduleUrl]);

  return { module, error, isLoading };
}

export default useWasmModule;