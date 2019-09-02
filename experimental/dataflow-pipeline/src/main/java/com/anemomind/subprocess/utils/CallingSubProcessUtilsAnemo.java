package com.anemomind.subprocess.utils;

import com.google.api.gax.paging.Page;
import com.google.cloud.storage.*;
import com.anemomind.subprocess.configuration.SubProcessConfiguration;
import org.apache.beam.vendor.guava.v20_0.com.google.common.collect.Sets;
import org.json.JSONArray;
import org.json.JSONObject;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.File;
import java.nio.file.Path;
import java.nio.file.Paths;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.Semaphore;

/**
 * Utility class for dealing with concurrency and binary file copies to the worker.
 */
public class CallingSubProcessUtilsAnemo {

    static final Logger LOG = LoggerFactory.getLogger(CallingSubProcessUtilsAnemo.class);
    // Allow multiple subclasses to create files, but only one thread per subclass can add the file to
    // the worker
    private static final Set<String> downloadedFiles = Sets.newConcurrentHashSet();
    static boolean initCompleted = false;
    // Limit the number of threads able to do work
    private static Map<String, Semaphore> semaphores = new ConcurrentHashMap<>();

    // Prevent Instantiation
    private CallingSubProcessUtilsAnemo() {
    }

    public static void setUp(SubProcessConfiguration configuration, String binaryName)
            throws Exception {
        {
            if (!semaphores.containsKey(binaryName)) {
                initSemaphore(configuration.getConcurrency(), binaryName);
            }

            synchronized (downloadedFiles) {
                if (!downloadedFiles.contains(binaryName)) {
                    // Create Directories if needed
                    FileUtils.createDirectoriesOnWorker(configuration);
                    LOG.info("Calling filesetup to move Executables to worker.");
                    ExecutableFile executableFile = new ExecutableFile(configuration, binaryName);
                    FileUtils.copyFileFromGCSToWorker(executableFile);
                    downloadedFiles.add(binaryName);
                }
            }
        }
    }

    public static String[] boatGcpToLocal(SubProcessConfiguration configuration, String bucketName, String pubsbumessage) {

        // Instantiates a client
        Storage storage = StorageOptions.getDefaultInstance().getService();
        String directoryPath = null;
        String processedDir = null;

        JSONObject jsonObject = new JSONObject(pubsbumessage);

        String directoryName = jsonObject.getString("boatDirectory");

        Page<Bucket> buckets = storage.list(Storage.BucketListOption.pageSize(100));

        for (Bucket bucket : buckets.iterateAll()) {

            if (bucket.getName().equals(bucketName)) {

                // listing all elements of given bucket.
                List<String> blobNames = new LinkedList<String>();
                JSONArray jsonArray = jsonObject.getJSONArray("files");
                for (int i = 0; i<jsonArray.length();i++){
                    blobNames.add(directoryName + "/" +jsonArray.getString(i));
                }

                List<Blob> blobs = bucket.get(blobNames);
                for(Blob blob : blobs){

                    if(blob != null){
                        if (blob.getName().split("/").length == 2) {

                            directoryPath = configuration.getWorkerPath() + directoryName;
                            processedDir = directoryPath + "/" + "processed";
                            File directory = new File(directoryPath);
                            File processDirectory = new File(processedDir);
                            // creating boat directory on worker node where all uploaded files gets downloaded.
                            if (!directory.exists())
                                directory.mkdir();
                            else
                                LOG.info("Directory " + directoryPath + " already exists...!");

                            // creating processed directory on worker node.
                            if (!processDirectory.exists())
                                processDirectory.mkdir();
                            else
                                LOG.info("Directory " + processedDir + " already exists...!");

                            String fileToDwnld = blob.getName().split("/")[1];
                            String localPath = configuration.getWorkerPath() + directoryName + "/" + fileToDwnld;

                            Path destpath = Paths.get(localPath);

                            Blob blobDwnl = storage.get(BlobId.of(bucket.getName(), blob.getName()));

                            System.out.println("Downloading file: " + fileToDwnld);
                            blobDwnl.downloadTo(destpath);
                        }
                    }else {
                        LOG.error("Given path " + blob.getName() + " is not available");
                    }
                }
            }
        }
        return new String[]{directoryPath, processedDir};
    }

    public static void soGcpToLocal(SubProcessConfiguration configuration, String bucketName) {

        // Instantiates a client
        Storage storage = StorageOptions.getDefaultInstance().getService();

        // get all available buckets
        Page<Bucket> buckets = storage.list(Storage.BucketListOption.pageSize(100));

        for (Bucket bucket : buckets.iterateAll()) {

            if (bucket.getName().equals(bucketName.trim())) {

                Page<Blob> blobs = bucket.list();

                for (Blob blob : blobs.iterateAll()) {

                    // All SO files are stored in libs directory of given bucket.
                    if (blob.getName().contains("libs/")) {

                        if (blob.getName().split("/").length == 2) {

                            // get each individual SO file
                            String fileToDwnld = blob.getName().split("/")[1];
                            String localPath = configuration.getWorkerPath() + "/" + fileToDwnld;

                            Path destpath = Paths.get(localPath); // worker path where the SO will downloaded.

                            Blob blobDwnl = storage.get(BlobId.of(bucket.getName(), blob.getName()));

                            if (blobDwnl != null) {

                                // downloading so file.
                                blobDwnl.downloadTo(destpath);

                            } else
                                LOG.error("Given path " + blob.getName() + " is not available");
                        }
                    }
                }
            }
        }
    }




    public static synchronized void initSemaphore(Integer permits, String binaryName) {
        if (!semaphores.containsKey(binaryName)) {
            LOG.info(String.format(String.format("Initialized Semaphore for binary %s ", binaryName)));
            semaphores.put(binaryName, new Semaphore(permits));
        }
    }

    private static void aquireSemaphore(String binaryName) throws IllegalStateException {
        if (!semaphores.containsKey(binaryName)) {
            throw new IllegalStateException("Semaphore is NULL, check init logic in @Setup.");
        }
        try {
            semaphores.get(binaryName).acquire();
        } catch (InterruptedException ex) {
            LOG.error("Interupted during aquire", ex);
        }
    }

    private static void releaseSemaphore(String binaryName) throws IllegalStateException {
        if (!semaphores.containsKey(binaryName)) {
            throw new IllegalStateException("Semaphore is NULL, check init logic in @Setup.");
        }
        semaphores.get(binaryName).release();
    }

    /**
     * Permit class for access to worker cpu resources.
     */
    public static class Permit implements AutoCloseable {

        private String binaryName;

        public Permit(String binaryName) {
            this.binaryName = binaryName;
            CallingSubProcessUtilsAnemo.aquireSemaphore(binaryName);
        }

        @Override
        public void close() {
            CallingSubProcessUtilsAnemo.releaseSemaphore(binaryName);
        }
    }
}
