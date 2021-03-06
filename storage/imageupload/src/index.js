const express = require("express");
const bodyParser = require("body-parser");
const chainLib = require("@blurtfoundation/blurtjs");
const {
  PrivateKey,
  PublicKey,
  Signature,
} = require("@blurtfoundation/blurtjs/lib/auth/ecc");
const RIPEMD160 = require("ripemd160");
const { RateLimiterMemory } = require("rate-limiter-flexible");
const { create } = require('ipfs-http-client')

// connect to ipfs daemon API server
const ipfs = create('http://localhost:5001') // (the default in Node.js)

let peers = async () => { return ipfs.swarm.addrs() }
let bw = async () => { return ipfs.stats.repo() }

bw().then(console.log)
peers().then(console.log)


chainLib.api.setOptions({
  url: process.env.JSONRPC_URL,
  retry: true,
  useAppbaseApi: true,
});


const rate_limit_opts = {
  points: 69, // 3 images
  duration: 3600, // per hour
};

const rateLimiter = new RateLimiterMemory(rate_limit_opts);

const app = express();

const port = process.env.PORT || 7070; // set our port

hdl_upload_s3 = async (req, res) => {
  try {
    const { username, sig } = req.params;

    // const username = this.session.a;
    if (username === undefined || username === null) {
      throw new Error("invalid user");
    }

    const jsonBody = req.body;
    // console.log(`jsonBody.data.length=${jsonBody.data.length}`);
    if (jsonBody.data.length > process.env.MAX_JSON_BODY_IN_BYTES) {
      throw new Error("File size too big!");
    }

    // data:image/jpeg;base64,
    let indexData = 0;
    if (jsonBody.data[23] === ",") {
      indexData = 23;
    } else if (jsonBody.data[22] === ",") {
      indexData = 22;
    } else if (jsonBody.data[21] === ",") {
      indexData = 21;
    } else {
      throw new Error("could not find index of [,]");
    }

    const prefix_data = jsonBody.data.substring(0, indexData);
    const base64_data = jsonBody.data.substring(indexData);

    // extract content type
    let file_ext = null;
    if (prefix_data.startsWith("data:image/jpeg;")) file_ext = "jpeg";
    else if (prefix_data.startsWith("data:image/jpg;")) file_ext = "jpg";
    else if (prefix_data.startsWith("data:image/png;")) file_ext = "png";
    else if (prefix_data.startsWith("data:image/gif;")) file_ext = "gif";
    else if (prefix_data.startsWith("data:image/webp;")) file_ext = "webp";
    else throw new Error("invalid content type");

    const content_type = `image/${file_ext}`;

    const buffer = new Buffer(base64_data, "base64");
    // console.log(`buffer.length=${buffer.length}`);
    if (buffer.length > process.env.MAX_IMAGE_SIZE_IN_BYTES) {
      throw new Error("File size too big!");
    }

    const hash_buffer = new RIPEMD160().update(buffer).digest("hex");
    const s3_file_path = `${username}/${hash_buffer}.${file_ext}`;

    {
      // verifying sig
      const isValidUsername = chainLib.utils.validateAccountName(username);
      if (isValidUsername) {
        throw new Error("Invalid username");
      }

      const existingAccs = await chainLib.api.getAccountsAsync([username]);
      if (existingAccs.length !== 1) {
        throw new Error("Invalid username.");
      }

      const sign_data = Signature.fromBuffer(new Buffer(sig, "hex"));
      const sigPubKey = sign_data.recoverPublicKeyFromBuffer(buffer).toString();

      const postingPubKey = existingAccs[0].posting.key_auths[0][0];
      const activePubKey = existingAccs[0].active.key_auths[0][0];
      const ownerPubKey = existingAccs[0].owner.key_auths[0][0];

      switch (sigPubKey) {
        case postingPubKey:
        case activePubKey:
        case ownerPubKey:
          // key matched, do nothing
          break;
        default:
          throw new Error("Invalid key.");
      }

      const is_verified = sign_data.verifyBuffer(
        buffer,
        PublicKey.fromString(sigPubKey)
      );
      if (!is_verified) {
        throw new Error("Invalid signature.");
      }
    }

    await rateLimiter.consume(username, 1);

    const { cid } = await ipfs.add(buffer);
    console.log(cid)	  


    const ipfs_full_path = `https://cloudflare-ipfs.com/ipfs/${cid}`;
    // this.body = JSON.stringify({status: 'ok', message: 'success', data: img_full_path});
    res.json({ status: "ok", message: "success", data: ipfs_full_path });
  } catch (e) {
    // console.error('Error in /imageupload api call', this.session.uid, error);
    res.json({ status: "error", message: e.message, data: e });
  }
};

// serverStart starts an express http server
serverStart = () => {
  app.use(bodyParser.json({ type: "application/json", limit: "10mb" }));

  const router = express.Router();
  router.post("/:username/:sig", hdl_upload_s3);
  router.get("/test_cors", async (req, res) => {
    res.json({ status: "ok", message: "success", data: null });
  });


router.get("/", function (req, res)  {
    peers().then(res.send)
    
  });

  app.use("/", router);

  app.listen(port);
  console.log(`serverStart on port ${port}`);
};

serverStart();

module.exports = app;
