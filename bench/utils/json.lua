-- example reporting script which demonstrates a custom
-- done() function that prints results as JSON

done = function(summary, latency, requests)
   io.write("\nJSON Output:\n")
   io.write("{\n")
   io.write(string.format("\t\"requests\": %d,\n", summary.requests))
   io.write(string.format("\t\"duration_in_microseconds\": %0.2f,\n", summary.duration))
   io.write(string.format("\t\"bytes\": %d,\n", summary.bytes))
   io.write(string.format("\t\"requests_per_sec\": %0.2f,\n", (summary.requests/summary.duration)*1e6))
   io.write(string.format("\t\"bytes_transfer_per_sec\": %0.2f,\n", (summary.bytes/summary.duration)*1e6))

   io.write("\t\"latency_distribution\": {\n")


--    for p, lat in ipairs(latency(100)) do
--     io.write(string.format("\t\t\"%d\": %g", p, lat))

--   end

    perc = {
        0.000000,
        0.100000,
        0.200000,
        0.300000,
        0.400000,
        0.500000,
        0.550000,
        0.600000,
        0.650000,
        0.700000,
        0.750000,
        0.775000,
        0.800000,
        0.825000,
        0.850000,
        0.875000,
        0.887500,
        0.900000,
        0.912500,
        0.925000,
        0.937500,
        0.943750,
        0.950000,
        0.956250,
        0.962500,
        0.968750,
        0.971875,
        0.975000,
        0.978125,
        0.981250,
        0.984375,
        0.985938,
        0.987500,
        0.989062,
        0.990625,
        0.992188,
        0.992969,
        0.993750,
        0.994531,
        0.995313,
        0.996094,
        0.996484,
        0.996875,
        0.997266,
        0.997656,
        0.998047,
        0.998242,
        0.998437,
        0.998633,
        0.998828,
        0.999023,
        0.999121,
        0.999219,
        0.999316,
        0.999414,
        0.999512,
        0.999561,
        0.999609,
        0.999658,
        0.999707,
        0.999756,
        0.999780,
        0.999805,
        0.999829,
        0.999854,
        0.999878,
        0.999890,
        0.999902,
        0.999915,
        0.999927,
        0.999939,
        0.999945,
        0.999951,
        0.999957,
        0.999963,
        0.999969,
        0.999973,
        0.999976,
        0.999979,
        0.999982,
        0.999985,
        0.999986,
        0.999988,
        0.999989,
        0.999991,
        0.999992,
        0.999993,
        0.999994,
        1.000000
    }

   for _, p in pairs(perc) do
        n = latency:percentile(p * 100)
        io.write(string.format("\t\t\"%f\": %g,\n", p, n))

    --   io.write("\t\t{\n")
    --   n = latency:percentile(p)
    --   io.write(string.format("\t\t\t\"percentile\": %g,\n\t\t\t\"latency_in_microseconds\": %d\n", p, n))
    --   if p == 100 then
    --       io.write("\t\t}\n")
    --   else
    --       io.write("\t\t},\n")
    --   end
   end
   io.write("\t]\n}\n")
end
