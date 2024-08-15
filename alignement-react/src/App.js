import React, { useEffect, useState } from 'react';

function App() {
  const [sequences, setSequences] = useState({ seq1: "", seq2: "" });
  const [alignment, setAlignment] = useState({ c1: "", c2: "" });
  const [similarityInfo, setSimilarityInfo] = useState({ max: 0, count: 0 });
  const [alignSequences, setAlignSequences] = useState(null); // Stocker la fonction alignSequences

  useEffect(() => {
    // Charger le module WebAssembly depuis le dossier public
    const loadWasmModule = async () => {
      try {
        const Module = await import('../public/common_molecular.js');
        const wasmModule = await Module();

        const alignSequencesFunction = (seq1, seq2) => {
          const length1 = seq1.length;
          const length2 = seq2.length;

          // Préparer les chaînes en mémoire
          const s1Ptr = wasmModule._malloc(length1 + 1);
          const s2Ptr = wasmModule._malloc(length2 + 1);
          wasmModule.stringToUTF8(seq1, s1Ptr, length1 + 1);
          wasmModule.stringToUTF8(seq2, s2Ptr, length2 + 1);

          // Allouer la matrice
          const matPtr = wasmModule._allocation(length1 + 1, length2 + 1);

          // Remplir la matrice
          wasmModule._FillMatrix(length1 + 1, length2 + 1, s1Ptr, s2Ptr, matPtr);

          // Récupérer les informations de similarité
          const maxInfo = wasmModule._get_max(length1 + 1, length2 + 1, matPtr);
          const max = wasmModule.getValue(maxInfo, 'double');
          const count = wasmModule.getValue(maxInfo + 8, 'i32'); // `cell_max` structure

          setSimilarityInfo({ max, count });

          // Effectuer le traceback pour obtenir l'alignement
          const tracebackResult = wasmModule._traceback(maxInfo, length1 + 1, length2 + 1, s1Ptr, s2Ptr, matPtr);

          // Libérer la mémoire allouée
          wasmModule._free(s1Ptr);
          wasmModule._free(s2Ptr);
          wasmModule._free(matPtr);

          // Décomposer l'alignement en deux chaînes distinctes
          const [c1, c2] = parseAlignment(tracebackResult, length1, length2, wasmModule);
          setAlignment({ c1, c2 });
        };

        // Stocker la fonction alignSequences dans l'état
        setAlignSequences(() => alignSequencesFunction);
      } catch (error) {
        console.error("Erreur lors du chargement du module WebAssembly:", error);
      }
    };

    loadWasmModule();
  }, []);

  // Fonction pour analyser le résultat de l'alignement
  const parseAlignment = (tracebackResult, length1, length2, wasmModule) => {
    // Récupérer les chaînes d'alignement en utilisant wasmModule.getValue ou d'autres méthodes pour accéder aux données
    // Cela dépend de la manière dont `_alignment` retourne les chaînes (via des pointeurs ou des structures complexes)

    // Exemple simplifié (à adapter selon le comportement exact du module C) :
    const c1 = wasmModule.UTF8ToString(tracebackResult); // Changer si nécessaire
    const c2 = wasmModule.UTF8ToString(tracebackResult + length1 + 1); // Ajustement nécessaire

    return [c1, c2];
  };

  return (
    <div className="App">
      <h1>Alignement de Séquences avec WebAssembly</h1>
      <div>
        <label>Séquence 1: </label>
        <input
          type="text"
          value={sequences.seq1}
          onChange={(e) => setSequences({ ...sequences, seq1: e.target.value })}
        />
      </div>
      <div>
        <label>Séquence 2: </label>
        <input
          type="text"
          value={sequences.seq2}
          onChange={(e) => setSequences({ ...sequences, seq2: e.target.value })}
        />
      </div>
      <button
        onClick={() => {
          if (alignSequences) {
            alignSequences(sequences.seq1, sequences.seq2);
          } else {
            console.error("La fonction alignSequences n'est pas encore chargée.");
          }
        }}
      >
        Aligner les séquences
      </button>

      <h2>Résultats</h2>
      <p>Maximal similarity degree: {similarityInfo.max}</p>
      <p>Occurrences: {similarityInfo.count}</p>
      <h3>Alignement:</h3>
      <pre>{alignment.c1}</pre>
      <pre>{alignment.c2}</pre>
    </div>
  );
}

export default App;
