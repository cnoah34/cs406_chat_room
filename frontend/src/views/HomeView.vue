<template>
    <div class="home">
        <Rooms class="left" />
        <Messages class="right" />
    </div>
</template>


<script setup>
    import Rooms from '../components/Rooms.vue'
    import Messages from '../components/Messages.vue'
</script>


<style scoped>
.home {
    display: flex;
    flex-direction: row;
    height: 85vh;
}

.home .left {
    height: 100%;
    min-width: 300px;
    border: 3px solid var(--vue-green);
}

.home .right {
    flex-grow: 1;
    height: 100%;
    border: 3px solid var(--vue-green);
    border-left: none;
}

</style>
